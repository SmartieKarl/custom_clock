#pragma once

#include "config.h"
#include "network.h" // For WeatherData type
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Hardware abstraction layer
// Centralizes all hardware object declarations and pin management
// Merged with startup functions for simplified file structure

// Hardware objects - declared here, defined in hardware.cpp
extern TFT_eSPI tft;
extern RTC_DS3231 rtc;
extern DFRobotDFPlayerMini player;
extern HardwareSerial mySoftwareSerial;
extern MFRC522 rfid;

// Main hardware initialization
void initializeHardware();
bool hardwareStartupOk();

// Individual hardware initialization functions (merged from startup.h)
bool initializeDisplay(TFT_eSPI &tft);
bool initializeDFPlayer(TFT_eSPI &tft, DFRobotDFPlayerMini &player, HardwareSerial &serial);
bool initializeRTC(TFT_eSPI &tft, RTC_DS3231 &rtc);
bool initializeRFID(TFT_eSPI &tft, MFRC522 &rfid);

// Network and data initialization
bool startupRTCSync(TFT_eSPI &tft);
bool startupWeatherFetch(TFT_eSPI &tft, WeatherData &weatherData);

// Startup flow control
void handleStartupError(TFT_eSPI &tft);
void completeStartup(TFT_eSPI &tft);

// Helper function for displaying startup status
void displayStartupStatus(TFT_eSPI &tft, const char *component, bool success, const char *message = "");