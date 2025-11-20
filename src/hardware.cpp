#include "hardware.h"
#include "alarm.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include <SPI.h>

// Hardware abstraction layer implementation
// Centralizes hardware object instantiation and initialization
// Merged with startup.cpp for simplified file structure

// Global hardware objects
TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;
DFRobotDFPlayerMini player;
HardwareSerial mySoftwareSerial(1); // UART1 for ESP32
MFRC522 rfid(RFID_CS_PIN, RFID_RST_PIN);

// Weather data storage
WeatherData currentWeather = {0, 0, 0, 0, "", "", false};

// Hardware initialization status
static bool startupOK = true;

// Initialize the ST7789 TFT display and show startup banner
bool initializeDisplay(TFT_eSPI &tft)
{
    tft.init();
    setTFTInstance(&tft);
    tft.setRotation(1); // Landscape orientation
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 5);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.println("- Michael's totally wicked custom clock v0.01 -");
    tft.println("");

    return true;
}

// Initialize the DFPlayer Mini audio module
bool initializeDFPlayer(TFT_eSPI &tft, DFRobotDFPlayerMini &player, HardwareSerial &serial)
{
    serial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

    tft.println("Initializing DFPlayer...");

    if (!player.begin(serial))
    {
        tft.setTextColor(TFT_RED);
        tft.println("DFPlayer failed!");
        tft.setTextColor(TFT_WHITE);
        return false;
    }
    else
    {
        player.volume(22); // Default volume 22
        return true;
    }
}

// Initialize the DS3231 RTC module
bool initializeRTC(TFT_eSPI &tft, RTC_DS3231 &rtc)
{
    tft.println("Initializing RTC...");\

    if (!rtc.begin())
    {
        tft.setTextColor(TFT_RED);
        tft.println("RTC failed!");
        tft.setTextColor(TFT_WHITE);
        return false;
    }
    else
    {
        if (rtc.lostPower())
        {
            // RTC lost power, reset date/time
            rtc.adjust(DateTime(0, 1, 1, 0, 0, 0));
            tft.setTextColor(TFT_YELLOW);
            tft.println("RTC lost power - time reset");
            tft.setTextColor(TFT_WHITE);
        }
        initializeAlarm(rtc);
        return true;
    }
}

// Initialize the MFRC522 RFID module
bool initializeRFID(TFT_eSPI &tft, MFRC522 &rfid)
{
    tft.println("Initializing RFID...");

    // Initialize SPI for RFID (shared with TFT)
    SPI.begin();

    // reset RFID module to ensure clean state
    digitalWrite(RFID_RST_PIN, HIGH);
    delay(10);
    digitalWrite(RFID_RST_PIN, LOW);
    delay(10);
    digitalWrite(RFID_RST_PIN, HIGH);
    delay(50);

    // Initialize RFID module
    rfid.PCD_Init();

    // Check if RFID module is responding
    byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

    if (version == 0x00 || version == 0xFF)
    {
        tft.setTextColor(TFT_RED);
        tft.println("RFID failed.");
        tft.setTextColor(TFT_WHITE);
        return false;
    }

    return true;
}

// Perform NTP synchronization
bool startupRTCSync(TFT_eSPI &tft)
{
    tft.println("Syncing time via NTP...");

    bool ntpResult = syncRTCFromNTP();

    if (!ntpResult)
    {
        tft.setTextColor(TFT_YELLOW);
        tft.println("RTC was unable to sync to NTP time.");
    }
    tft.setTextColor(TFT_WHITE);

    return ntpResult;
}

// Fetch initial weather data
bool startupWeatherFetch(TFT_eSPI &tft, WeatherData &weatherData)
{
    tft.println("Fetching weather...");

    bool weatherResult = fetchWeather(weatherData);

    if (!weatherResult)
    {
        tft.setTextColor(TFT_YELLOW);
        tft.println("Weather data not obtained.");
    }
    tft.setTextColor(TFT_WHITE);

    return weatherResult;
}

// ========== MAIN HARDWARE INITIALIZATION ==========

bool initializeHardware()
{
    // Load persistent user settings from flash
    loadUserSettings();

    // Initialize display and show startup banner
    initializeDisplay(tft);

    // Initialize brightness control
    initializeBrightness();

    // Initialize hardware components
    if (!initializeDFPlayer(tft, player, mySoftwareSerial))
    {
        startupOK = false;
    }
    if (!initializeRTC(tft, rtc))
    {
        startupOK = false;
    }
    if (!initializeRFID(tft, rfid))
    {
        startupOK = false;
    }
    if (!startupOK)
    {


        tft.setTextColor(TFT_RED);
        tft.println("Oh no! Something isn't responding.");
        tft.println("Check hardware connections.");
        tft.setTextColor(TFT_WHITE);
        return false;
    }

    startupRTCSync(tft);
    startupWeatherFetch(tft, currentWeather);
    return true;
}