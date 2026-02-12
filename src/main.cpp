/****************************
 * Alarm Clock Project
 * Written by Michael Marsh
 *
 * Hardware: ESP32 + ST7789 TFT + DS3231 RTC + DFPlayer Mini + RFID-RC522
 ****************************/

#include "brightness.h"
#include "button.h"
#include "command_interface.h"
#include "config.h"
#include "display.h"
#include "log.h"
#include "network.h"
#include "player.h"
#include "rfid.h"
#include "rtc.h"
#include "settings.h"
#include "tasks.h"
#include <esp_task_wdt.h>

// Helpers
bool initializeHardware();
void handleClockButtons(ButtonState butIn);
void handleSettingsButtons(ButtonState butIn);
void handleSerialCommand(const char *cmd);

// App state
AppMode currentMode = AppMode::BOOT;

//========== ENTRY POINT ==========

void setup()
{
    Serial.begin(9600);

    uSet.load();

    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);

    initWiFiManager();
    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");

    // Initialize hardware
    if (!initializeHardware())
    {
        unsigned long lastAction = millis();
        const unsigned long interval = 1000;
        tftPrintText("Press button 1 to continue", TFT_YELLOW);
        while (!getButtonStates().btn1)
        {
            unsigned long now = millis();
            if (now - lastAction >= interval)
            {
                tftPrintText(".", TFT_YELLOW);
                lastAction = now;
            }
            delay(10);
        }
    }
    else
    {
        delay(1000);
    }

    esp_reset_reason_t reason = esp_reset_reason();
    LOG.log("\nREBOOT, code %d", int(reason));

    currentMode = AppMode::CLOCK;
    drawClockScreen(getNow(), currentWeather);

    // Start background tasks
    xTaskCreatePinnedToCore(weatherTask, "WeatherTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(timeSyncTask, "TimeSyncTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(blynkTask, "BlynkTask", 16384, NULL, 1, NULL, 1);
}

void loop()
{
    static unsigned long alarmStartMillis = 0;
    static bool snoozeAlarmTriggered = false;
    static bool isColon = false;

    static uint8_t lastSecond = 255;
    static uint8_t lastMinute = 255;
    static uint8_t lastDay = 255;
    static ButtonState lastButtonState = {false, false, false, false};
    static unsigned long lastBrightnessUpdate = 0;
    static char inputBuffer[64];
    static uint8_t inputPos = 0;

    esp_task_wdt_reset(); // feed the dog

    DateTime now = getNow();
    uint8_t currentSecond = now.second();
    uint8_t currentMinute = now.minute();
    uint8_t currentDay = now.day();
    ButtonState currentButtonState = getButtonStates();

    // Check for serial input
    handleSerialIn();

    // Buttons
    bool buttonJustPressed = currentButtonState.any() && !lastButtonState.any();
    if (buttonJustPressed)
    {
        handleClockButtons(currentButtonState);
    }
    lastButtonState = currentButtonState;

    // Clock mode operations
    if (currentMode == AppMode::CLOCK)
    {
        // Check for alarm card scan
        if (isAlarmRinging())
        {
            RFIDResult rfidResult = rfidCheckCard();
            if (rfidResult.event != RFIDEvent::NONE)
            {
                setAlarmRinging(false);
                stopAudioPlayback();
                setVolume(PLAYER_VOLUME);
                flashScreen(TFT_GREEN, 500);
                drawClockScreen(now, currentWeather);

                // reset snooze logic
                alarmStartMillis = 0;
                snoozeAlarmTriggered = false;
                return;
            }

            if (!snoozeAlarmTriggered && millis() - alarmStartMillis >= 5UL * 60UL * 1000UL)
            {
                // setVolume(25);
                // loopPlayerFolder(2);
                playTrack(3, 2, 25);
                snoozeAlarmTriggered = true;
            }
        }

        // Daily updates
        if (currentDay != lastDay)
        {
            lastDay = currentDay;
            updateDateDisplay(now);
        }

        // Minutely updates
        if (currentMinute != lastMinute)
        {
            lastMinute = currentMinute;

            // Check alarm
            if (isAlarmTime())
            {
                alarmStartMillis = millis();

                updateAlarmDisplay();
                playTrack(random(1, 9), 1, PLAYER_VOLUME);
            }
        }

        // Per-second updates
        if (currentSecond != lastSecond)
        {
            lastSecond = currentSecond;
            isColon = !isColon;
            updateTimeDisplay(now, isColon);
        }
    }

    // Update ambient brightness
    if (millis() - lastBrightnessUpdate >= 100)
    {
        lastBrightnessUpdate = millis();
        updateAmbientBrightness();
    }

    vTaskDelay(pdMS_TO_TICKS(1));
}

//========== BUTTON HANDLERS ==========

void handleClockButtons(ButtonState butIn)
{
    // Two alarms
    static AlarmTime alarms[] = {
        {6, 0, true}, // 6am
        {7, 0, true}, // 7am
        {8, 0, true}, // 8am
        {9, 0, true}, // 9am
    };

    constexpr size_t ALARM_COUNT = sizeof(alarms) / sizeof(alarms[0]);
    static size_t alarmIndex = 0; // which alarm is selected

    static int testTrack = 1;

    if (!butIn.singlePress())
        return;

    if (butIn.btn1)
    {
        // Previous alarm
        alarmIndex = (alarmIndex + ALARM_COUNT - 1) % ALARM_COUNT;
        AlarmTime alm = alarms[alarmIndex];
        setAlarm(alm.hour, alm.minute, alm.enabled);
        updateAlarmDisplay();

        Serial.print("Selected alarm: ");
        Serial.println(alarmIndex);
    }
    else if (butIn.btn2)
    {
        // Next alarm
        alarmIndex = (alarmIndex + 1) % ALARM_COUNT;
        AlarmTime alm = alarms[alarmIndex];
        setAlarm(alm.hour, alm.minute, alm.enabled);
        updateAlarmDisplay();

        Serial.print("Selected alarm: ");
        Serial.println(alarmIndex);
    }
    else if (butIn.btn3)
    {
        // Placeholder for future function
        testTrack = (testTrack + 1) % 4;
        playTrack(testTrack, 2);
        Serial.print("Now playing track ");
        Serial.println(testTrack);
    }
    else if (butIn.btn4)
    {
        AlarmTime alm = getAlarm();
        bool newState = !alm.enabled;
        setAlarm(alm.hour, alm.minute, newState);
        updateAlarmDisplay();
        Serial.println(alm.enabled ? "Alarm enabled" : "Alarm disabled");
    }
}

//========== HARDWARE INITIALIZATION ==========

bool initializeHardware()
{
    bool rtcOK = true;
    bool playerOK = true;
    bool rfidOK = true;

    // Initialize components
    initializeButtons();
    initializeDisplay();
    initializeBrightness();

    if (!initializeRTC())
    {
        rtcOK = false;
        Serial.println("ERROR: RTC initialization failed");
    }

    if (!initializeDFPlayer(mySoftwareSerial))
    {
        playerOK = false;
        Serial.println("ERROR: DFPlayer initialization failed");
    }

    if (!initializeRFID())
    {
        rfidOK = false;
        Serial.println("ERROR: RFID initialization failed");
    }

    return displayStartupStatus(rtcOK, playerOK, rfidOK);
}