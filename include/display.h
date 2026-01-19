#pragma once
#include "domain.h"
#include "config.h"
#include <TFT_eSPI.h>
#include <RTClib.h>
#include <stdint.h>

//Display module
//Handles all display-related functions and TFT communication

extern TFT_eSPI tft;


//init
bool initializeDisplay();

//helpers
void drawCenteredString(const char text[],
    uint16_t textColor = TEXT_COLOR,
    uint16_t bgColor = BACKGROUND_COLOR,
    uint8_t font = 1,
    uint8_t size = 2);
void drawButtonLabel(uint8_t butNum,
    const char label[],
    uint16_t textColor = TEXT_COLOR,
    uint16_t bgColor = BACKGROUND_COLOR,
    uint8_t font = 1,
    uint8_t size = 1);
void flashScreen(uint16_t flashColor, int flashDuration);

//screen updaters
void updateTimeDisplay(const DateTime &now);
void updateDateDisplay(const DateTime &now);
void updateWeatherDisplay(const WeatherData &weather);
void updateAlarmDisplay();

//screen modes
void drawClockScreen(const DateTime &now, const WeatherData &weather);
void drawSettingsMenu(const char labCent[],
    const char lab1[],
    const char lab2[],
    const char lab3[],
    const char lab4[],
    uint16_t textColor = TEXT_COLOR,
    uint16_t bgColor = BACKGROUND_COLOR);

bool displayStartupStatus(bool rtcOK, bool playerOK, bool rfidOK);

void tftPrintLine(const char *message, uint16_t color);
void tftPrintText(const char *message, uint16_t color);