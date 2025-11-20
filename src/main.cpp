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
    DateTime now = rtc.now();

    // Initialize all hardware
    if (!initializeHardware())
    {
        tft.println("");
        tft.setTextColor(TFT_PINK);
        tft.println("---------------Congrations, you broke it---------------");
        while (true)
        {
            delay(1000); // If you got here, check wiring?
        }
    }

    delay(100);
    
    // Clear screen
    tft.fillScreen(TFT_BLACK);
    updateTimeDisplay(now);
    updateDateDisplay(now);
}

void loop()
{
    static uint8_t lastSecond = 255;
    static uint8_t lastMinute = 255;
    static uint8_t lastDay = 255;

    DateTime now = rtc.now();
    uint8_t currentSecond = now.second();
    uint8_t currentMinute = now.minute();
    uint8_t currentDay = now.day();
    

    // Check for RFID cards
    checkRFIDCard(rfid, player, rtc, currentWeather);

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
        checkAlarmTime(rtc, player);

        // Update weather every 30 minutes
        if (currentMinute == 0 || currentMinute == 30)
        {
            bool result = fetchWeather(currentWeather);
            if (result)
            {
                updateWeatherDisplay(currentWeather);
            }
        }
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