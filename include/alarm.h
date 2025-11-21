#pragma once

#define DS3231_ADDRESS 0x68
#define DS3231_CONTROL 0x0E

#include "config.h"
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Clock module for ESP32 alarm clock project
// Handles alarm functions



// ---------- Alarm module ----------

struct AlarmTime
    {
        uint8_t hour; // hour
        uint8_t minute; // minute
        bool enabled; // is alarm enabled
    };

// Global alarm state (declared here, defined in alarm.cpp)
extern bool alarmRinging;

// Alarm control functions
void initializeAlarm(RTC_DS3231 &rtc);
bool checkAlarmTime(RTC_DS3231 &rtc, DFRobotDFPlayerMini &player);
void stopAlarm(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc);

// Alarm time management functions
AlarmTime getAlarm();
void setAlarm(RTC_DS3231 &rtc, uint8_t hr, uint8_t min, bool enable);
// Alarm status functions
bool isAlarmEnabled();
bool isAlarmRinging();