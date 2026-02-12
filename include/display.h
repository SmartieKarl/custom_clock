#pragma once
#include "config.h"
#include "domain.h"
#include <RTClib.h>
#include <TFT_eSPI.h>
#include <stdint.h>

// Display module
// Handles all display-related functions and TFT communication

// init
bool initializeDisplay();

// helpers
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

void tftPrintLine(const char *message, uint16_t color);
void tftPrintText(const char *message, uint16_t color);

void flashScreen(uint16_t flashColor, int flashDuration);

// ========== BOOT MODE ==========
bool displayStartupStatus(bool rtcOK, bool playerOK, bool rfidOK);

// ========== CLOCK MODE ==========
void drawClockScreen(const DateTime &now, const WeatherData &weather);

void updateTimeDisplay(const DateTime &now, bool isColon = true);
void updateDateDisplay(const DateTime &now);
void updateWeatherDisplay(const WeatherData &weather);
void updateAlarmDisplay();

// ========== SETTINGS MODE ==========
void drawSettingsMenu(const char labCent[],
                      const char lab1[],
                      const char lab2[],
                      const char lab3[],
                      const char lab4[],
                      uint16_t textColor = TEXT_COLOR,
                      uint16_t bgColor = BACKGROUND_COLOR);