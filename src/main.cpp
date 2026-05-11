#include "AlarmSystem.h"
#include "AppController.h"
#include "BrightnessController.h"
#include "Buttons.h"
#include "CommandInterface.h"
#include "Config.h"
#include "Log.h"
#include "NetworkManager.h"
#include "RFIDHandler.h"
#include "Timekeeper.h"
#include "UI.h"
#include <DFRobotDFPlayerMini.h> // DFPlayer Mini core library
#include <HardwareSerial.h>      // Serial comms for DFPlayer Mini
#include <MFRC522.h>             // RFID core library
#include <RTClib.h>              // DS3231 RTC core library
#include <TFT_eSPI.h>            // ST7789 TFT core library
#include <esp_task_wdt.h>        // For freeRTOS task implementation

// Hardware objects
DFRobotDFPlayerMini player;
HardwareSerial mySoftwareSerial(1); // UART1 for ESP32 (DFPlayer)
MFRC522 rfid(RFID_CS_PIN, RFID_RST_PIN);
RTC_DS3231 rtc;
TFT_eSPI tft = TFT_eSPI();

// Logic objects
Buttons btn(BUTTON_1_PIN, BUTTON_2_PIN, BUTTON_3_PIN, BUTTON_4_PIN); // TODO: add default pins into constructor
BrightnessController brightness;                                     // uses default pins from Config.h
Timekeeper timekeeper(rtc);
NetworkManager networkManager(rtc);
RFIDHandler rfidHandler(rfid);
AlarmSystem alarmSystem(rtc, timekeeper, player);
UI ui(tft, btn, timekeeper, networkManager);
AppController appController(btn, rfidHandler, alarmSystem, ui, player);

CommandInterface commandInterface(player, timekeeper, ui, networkManager, alarmSystem);

//========== INITIALIZATION ==========
struct HardwareStatus
{
    bool rtcOK = true;
    bool rtcLostPower = false;
    bool playerOK = true;
    bool rfidOK = true;
};

void setupSys()
{
    // uSet.load(); // Load user settings from EEPROM (TODO: make this functional)

    // Watchdog
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
}

HardwareStatus setupBoard()
{
    HardwareStatus hs;

    //===== TFT init =====
    tft.init();
    tft.fillScreen(BACKGROUND_COLOR);

    //===== Brightness controller init =====
    // Must come after tft.init() so the PWM pin doesn't fight the TFT setup
    brightness.begin();

    //===== Buttons init =====
    btn.begin();  // creates Buttons mutex and sets pinMode
    btn.update(); // takes an initial reading so getState() is valid

    //===== RTC init =====
    if (!rtc.begin())
        hs.rtcOK = false;

    if (rtc.lostPower())
    {
        hs.rtcLostPower = true;
        rtc.adjust(DateTime(0, 1, 1, 0, 0, 0));
        rtc.setAlarm1(DateTime(0, 0, 0, 0, 0, 0), DS3231_A1_Hour);
        rtc.disableAlarm(1);
    }

    //===== Timekeeper init =====
    timekeeper.begin(); // creates Timekeeper mutex

    //===== DFPlayer init =====
    mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
    delay(1000);                                      // Wait for .begin() to succeed

    if (!player.begin(mySoftwareSerial))
        hs.playerOK = false;
    else
        player.volume(PLAYER_VOLUME);

    //===== RFID init =====
    SPI.begin();
    rfid.PCD_Init();

    byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF)
        hs.rfidOK = false;

    //===== NetworkManager init =====
    networkManager.begin(); // creates mutex for network access

    return hs;
}

//========== HELPERS ==========
void logResetReason()
{
    esp_reset_reason_t reason = esp_reset_reason();
    LOG.log("\nREBOOT, code %d", int(reason));
}

void waitForUserContinue()
{
    unsigned long lastAction = millis();
    const unsigned long interval = 1000;

    tft.setTextColor(TFT_YELLOW);
    tft.print("Press any button to continue");

    while (!btn.getState().anyPressed())
    {
        unsigned long now = millis();
        if (now - lastAction >= interval)
        {
            tft.print(".");
            lastAction = now;
        }
        btn.update();
        delay(10);
    }
    tft.setTextColor(TEXT_COLOR);
}

// FreeRTOS tasks funciton definitions
void timeSyncTask(void *);
void weatherTask(void *);
void blynkTask(void *);

//==================== ENTRY POINT FOR PROGRAM ====================
void setup()
{
    Serial.begin(9600); // TOGO: fix serial?

    setupSys();
    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");

    HardwareStatus hs = setupBoard();

    alarmSystem.begin();
    ui.begin();

    //========== Callback registration ==========
    alarmSystem.onAlarmEvent([&]()
                             { ui.updateAlarmDisplay(); });

    ui.setAlarmDataCallback([&]() -> UI::AlarmDisplayData
                            {
            AlarmTime a = alarmSystem.getAlarm();
            return { a.hour, a.minute, a.enabled, alarmSystem.isRinging() }; });

    rfidHandler.onRFIDEvent([&](const char *uid)
                            {
            if (alarmSystem.isRinging())
            {
                alarmSystem.dismissAlarm();
                ui.flashScreen(TFT_GREEN);
            } });

    // Display hardware notification if available
    if (!ui.displayStartupStatus(hs.rtcOK, hs.rtcLostPower, hs.playerOK, hs.rfidOK))
        waitForUserContinue();
    else
        delay(3000);

    xTaskCreatePinnedToCore(timeSyncTask, "TimeSyncTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(weatherTask, "WeatherTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(blynkTask, "BlynkTask", 16384, NULL, 1, NULL, 1);

    ui.setState(State::Clock);
}

void loop()
{
    timekeeper.update();
    btn.update();

    rfidHandler.poll();

    alarmSystem.run();
    ui.run();

    appController.handleIn();

    brightness.updateAmbient();

    commandInterface.handleSerialIn();

    esp_task_wdt_reset();
    delay(10);
}

//==================== FreeRTOS Tasks ====================
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

// Blynk callback functions here

// Command-response virtual pin
BLYNK_WRITE(V0)
{
    Serial.println("BLYNK_WRITE: started");
    const char *rsp = commandInterface.handleBlynkIn(param.asStr());
    if (rsp[0] != '\0') // Will be '/0' if the read command had the signature clock prefix: [CLK]:
    {
        // Write and notify
        Blynk.virtualWrite(V0, rsp);
        Blynk.logEvent("clock_reply", rsp);
        Serial.printf("rsp <%s> written to pin V0.", rsp);
    }
}

// On blynk connect, flush log to log pin (V1)
BLYNK_CONNECTED()
{
    Serial.println("BLYNK_CONNECTED: started.");
    Blynk.sendCmd(BLYNK_CMD_PING); // Outbound ping to prevent server-side idle timeout
    Blynk.syncVirtual(V0);         // Syncs last value from cmd stream for execution
    while (LOG.hasLog())
    {
        // Write to V1 (log stream)
        const char *lPop = LOG.pop();
        Blynk.virtualWrite(V1, lPop);
        Blynk.logEvent("clock_log", lPop);
        vTaskDelay(50); // to avoid flooding Blynk
    }
}

void timeSyncTask(void *)
{
    while (true)
    {
        networkManager.syncRTCFromNTP();

        DateTime now = timekeeper.time();
        uint32_t secondsToMidnight = (23 - now.hour()) * 3600 +
                                     (59 - now.minute()) * 60 +
                                     (60 - now.second());

        vTaskDelay(pdMS_TO_TICKS(secondsToMidnight * 1000ULL));
    }
}

void weatherTask(void *)
{
    while (true)
    {
        if (networkManager.fetchWeather())
            ui.updateWeatherDisplay(networkManager.currentWeather);

        DateTime now = timekeeper.time();
        uint32_t secondsToNext = 0;
        if (now.minute() < 30)
        {
            secondsToNext = (30 - now.minute()) * 60 - now.second();
        }
        else
        {
            secondsToNext = (60 - now.minute()) * 60 - now.second();
        }

        vTaskDelay(pdMS_TO_TICKS(secondsToNext * 1000ULL));
    }
}

void blynkTask(void *)
{
    Serial.println("BlynkTask: started");
    constexpr int MAX_RETRIES = 5;
    constexpr unsigned long BURST_MS = 20000; // 20s
    Blynk.config(BLYNK_AUTH);

    while (true)
    {
        bool persistent = networkManager.isWiFiPersistent();

        // Connect to WiFi (retry connection if periodic)
        bool connected = false;
        int retries = persistent ? 1 : MAX_RETRIES;

        for (int i = 0; i < retries && !connected; i++)
        {
            Serial.println("BlynkTask: attempting to connect to WiFi");
            if (networkManager.startWiFiSession())
            {
                Serial.println("BlynkTask: WiFi connected successfully.");
                connected = true;
            }
            else if (!persistent)
            {
                Serial.printf("BlynkTask: WiFi failed. Retrying... (%d/%d)\n", i + 1, MAX_RETRIES);
            }
        }

        // If connected, establish Blynk connection
        if (connected)
        {
            Blynk.connect(10000);
            if (Blynk.connected())
            {
                // Run Blynk
                if (persistent)
                {
                    // Persistent mode: run continuously until persistence ends
                    while (networkManager.isWiFiPersistent())
                    {
                        Blynk.run();
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                    Serial.println("BlynkTask: Persistence ended, closing WiFi.");
                }
                else
                {
                    // Burst mode: run for 10 seconds then disconnect
                    unsigned long start = millis();
                    while (millis() - start < BURST_MS)
                    {
                        Blynk.run();
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                }
                Blynk.disconnect();
                networkManager.endWiFiSession();
                Serial.println("BlynkTask: WiFi session ended.");
            }
        }
        else
            LOG.log("Failed to connect to Blynk cloud.");

        // After disconnect / failed connect
        DateTime now = timekeeper.time();
        uint32_t secondsToNext = 0;
        if (now.minute() < 30)
        {
            secondsToNext = (30 - now.minute()) * 60 - now.second();
        }
        else
        {
            secondsToNext = (60 - now.minute()) * 60 - now.second();
        }

        unsigned long waitStart = millis();
        unsigned long waitMs = secondsToNext * 1000ULL;
        while (millis() - waitStart < waitMs)
        {
            if (networkManager.isWiFiPersistent())
                break;                       // Exit early if persistence requested
            vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
        }
    }
}