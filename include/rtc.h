#pragma once
#include <RTClib.h>
#include "domain.h"

//RTC module
//Controls the RTC hardware object and all related functions

bool initializeRTC();

//Network functions
bool syncRTCFromNTP();

//Alarm functions
void initializeRTCAlarm();
bool isAlarmTime();
void setAlarm(uint8_t hr, uint8_t min, bool enable);
bool isAlarmEnabled();

bool isAlarmRinging();
void setAlarmRinging(bool ringing);
AlarmTime getAlarm();

//Utility functions
DateTime getNow();