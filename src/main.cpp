/****************************
 * Alarm Clock Project
 * Written by Michael Marsh
 *
 * Hardware: ESP32 + ST7789 TFT + DS3231 RTC + DFPlayer Mini + RFID-RC522
 * See /README for details on hardware and pin connections
 ****************************/

// ========== Includes ==========
#include "brightness.h"
#include "button.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include "player.h"
#include "rfid.h"
#include "rtc.h"
#include "settings.h"

// FreeRTOS tasks
void weatherTask(void *);
void timeSyncTask(void *);

// helpers
bool initializeHardware();

//========== ENTRY POINT FOR PROGRAM ==========

void setup()
{
    // Load persistent user settings from flash
    uSet.load();

    initWiFiManager();

    if (!initializeHardware())
    {
        tftPrintLine("Continue anyways?", TFT_YELLOW);
        while(!getButtonStates().any())
        {
            delay(10);
        }
    }
    else delay(1000);

    drawMainScreen(getNow(), currentWeather);

    xTaskCreate(
        weatherTask,
        "WeatherTask",
        4096,
        NULL,
        0,
        NULL);
    xTaskCreate(
        timeSyncTask,
        "TimeSyncTask",
        4096,
        NULL,
        0,
        NULL);
}

void loop()
{
    static uint8_t lastSecond = 255;
    static uint8_t lastMinute = 255;
    static uint8_t lastDay = 255;

    DateTime now = getNow();
    uint8_t currentSecond = now.second();
    uint8_t currentMinute = now.minute();
    uint8_t currentDay = now.day();

    // Check for RFID cards
    rfidCheckCard();

    // Check for button press
    // handleButtonInput(rtc, player); //depto be fixedrecated, needs

    // Tasks per day
    if (currentDay != lastDay)
    {
        lastDay = currentDay;

        updateDateDisplay(now);
    }

    // Tasks per minute
    if (currentMinute != lastMinute)
    {
        lastMinute = currentMinute;

        // Check if it's alarm time
        checkIfAlarmTime();
    }

    // Tasks per second
    if (currentSecond != lastSecond)
    {
        lastSecond = currentSecond;

        updateTimeDisplay(now);
    }

    // Update screen brightness
    updateAmbientBrightness();

    delay(LOOP_DELAY);
}

//========== FREERTOS TASKS ==========

// Syncs weather data from online API on startup and every 30 minutes
void weatherTask(void *parameter)
{
    while (true)
    {
        if (fetchWeather(currentWeather))
            updateWeatherDisplay(currentWeather);

        DateTime now = getNow();
        uint8_t minutes = now.minute();
        uint8_t seconds = now.second();
        int next = (minutes < 30) ? 30 : 60;
        uint16_t waitSeconds = (next - minutes) * 60 - seconds;
        vTaskDelay(pdMS_TO_TICKS(waitSeconds * 1000UL)); // delay task until next half hour
    }
}

// Syncs time from NTP server on startup and every midnight afterwards
void timeSyncTask(void *parameter)
{
    while (true)
    {
        syncRTCFromNTP();

        DateTime now = getNow();
        uint8_t minutes = now.minute();
        uint8_t seconds = now.second();
        uint32_t secondsToMidnight = (23 - now.hour()) * 3600 + (59 - now.minute()) * 60 + (60 - now.second());
        vTaskDelay(pdMS_TO_TICKS(secondsToMidnight * 1000UL)); // delay task until midnight
    }
}

//========== HELPERS ==========
// Initializes all hardware components
bool initializeHardware()
{
    bool rtcOK = true;
    bool playerOK = true;
    bool rfidOK = true;

    // Initialize button inputs

    // Initialize display and show startup banner
    initializeDisplay();

    // Initialize brightness control
    initializeBrightness();

    // Initialize hardware components
    if (!initializeRTC())
    {
        rtcOK = false;
    }
    if (!initializeDFPlayer(player, mySoftwareSerial))
    {
        playerOK = false;
    }
    if (!initializeRFID())
    {
        rfidOK = false;
    }

    return displayStartupStatus(rtcOK, playerOK, rfidOK);
}