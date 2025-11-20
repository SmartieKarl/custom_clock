#pragma once

#include "config.h"
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Clock module for ESP32 alarm clock project
// Handles alarm functions



// ---------- Alarm module ----------
// Alarm time structure
struct AlarmTime
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

// Global alarm state (declared here, defined in alarm.cpp)
extern bool alarmRinging;
extern AlarmTime currentAlarm;

// Alarm control functions
void initializeAlarm(RTC_DS3231 &rtc);
bool checkAlarmTime(RTC_DS3231 &rtc, DFRobotDFPlayerMini &player);
void stopAlarm(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc);

// Alarm time management functions
void setAlarmTime(RTC_DS3231 &rtc, uint8_t hour, uint8_t minute);
AlarmTime getAlarmTime();
void enableAlarm(RTC_DS3231 &rtc, bool enable);

// Alarm status functions
bool isAlarmRinging();