#pragma once

#include "alarm.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Pure RFID module for ESP32 alarm clock project
// Handles RFID card detection, card actions, and card management
// Hardware initialization is handled by startup module

// Main RFID functions
void checkRFIDCard(MFRC522 &rfid, DFRobotDFPlayerMini &player, RTC_DS3231 &rtc, WeatherData &weather);

// Card action handlers
void handleAlarmCard(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc, WeatherData &weather);
void handleUnknownCard(const String &cardUID, RTC_DS3231 &rtc, WeatherData &weather);

// RFID utility functions
String getCardUID(MFRC522 &rfid);
void restoreDisplay(RTC_DS3231 &rtc, const WeatherData &weather);