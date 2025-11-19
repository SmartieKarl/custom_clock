#pragma once

#include "config.h"
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Alarm module for ESP32 alarm clock project
// Handles alarm timing, triggering, snoozing, and time management

// Alarm time structure for easy management
struct AlarmTime
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

// Global alarm state (declared here, defined in alarm.cpp)
extern bool alarmTriggered;
extern uint8_t currentAlarmIndex;
extern AlarmTime alarmTimes[];
extern const uint8_t NUM_ALARM_TIMES;

// Alarm control functions
void initializeAlarm();
bool checkAlarmTime(const DateTime &now, DFRobotDFPlayerMini &player);
void maintainAlarm(DFRobotDFPlayerMini &player);
void stopAlarm(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc);
void resetAlarmFlag(const DateTime &now, DFRobotDFPlayerMini &player);

// Alarm time management functions
void cycleAlarmTime();
AlarmTime getCurrentAlarmTime();

// Alarm status functions
bool isAlarmTriggered();