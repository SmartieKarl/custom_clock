/****************************
 * Alarm Clock Project
 * Written by Michael Marsh
 *
 * Hardware: ESP32 + ST7789 TFT + DS3231 RTC + DFPlayer Mini + RFID-RC522
 * See /README for details on hardware and pin connections
 ****************************/

// ========== Includes ==========
#include "alarm.h"
#include "config.h"
#include "display.h"
#include "hardware.h"
#include "network.h"
#include "rfid.h"
#include <Arduino.h>

// External weather data (managed by hardware layer)
extern WeatherData currentWeather;

// ========== Entry Points for Arduino ==========
void setup()
{
    // Initialize all hardware
    initializeHardware();

    // Force initial display updates
    DateTime now = rtc.now();
    updateTimeDisplay(now);
    updateDateDisplay(now);
    updateWeatherDisplay(currentWeather);
    updateAlarmDisplay();
}

void loop()
{
    static uint8_t lastSecond = 255; // Force first update
    static uint8_t lastMinute = 255; // Track minute changes for weather updates

    DateTime now = rtc.now();
    uint8_t currentSecond = now.second();
    uint8_t currentMinute = now.minute();

    // Check for RFID cards
    checkRFIDCard(rfid, player, rtc, currentWeather);

    // Handle alarm functions
    checkAlarmTime(now, player);
    maintainAlarm(player);
    resetAlarmFlag(now, player);

    // Update weather every 30 minutes
    if (currentMinute != lastMinute)
    {
        lastMinute = currentMinute;
        if (currentMinute == 0 || currentMinute == 30)
        {
            bool result = fetchWeather(currentWeather);
            if (result)
            {
                updateWeatherDisplay(currentWeather);
            }
        }
    }

    // Update time display every second
    if (currentSecond != lastSecond)
    {
        lastSecond = currentSecond;
        updateTimeDisplay(now);
        updateDateDisplay(now);
    }

    // Update alarm display when needed
    if (alarmNeedsUpdate)
    {
        updateAlarmDisplay();
        setAlarmUpdateFlag(false);
    }

    // Update screen brightness
    updateAmbientBrightness();

    delay(LOOP_DELAY);
}