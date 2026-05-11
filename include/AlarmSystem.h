#pragma once
#include "Config.h"
#include "Timekeeper.h"
#include <DFRobotDFPlayerMini.h>
#include <RTClib.h>
#include <functional>
#include <stdint.h>

// AlarmSystem.h
// Central alarm control system

struct AlarmTime
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

class AlarmSystem
{
  public:
    AlarmSystem(RTC_DS3231 &rtc, Timekeeper &tk, DFRobotDFPlayerMini &player);

    void begin();

    void run();

    AlarmTime getAlarm();
    void setAlarm(uint8_t hr, uint8_t min, bool enable);

    void dismissAlarm();

    bool isRinging();
    bool isAlarmTime();

    //========== Callbacks ==========
    using Callback = std::function<void()>;
    // Registers a callback when alarm has an event
    // (e.g., alarm changes or alarm triggers)
    void onAlarmEvent(Callback cb) { _onAlarmEvent = cb; };

    //========== Alarm config ==========
    // Track range for alarm sounds
    std::pair<uint8_t, uint8_t> trackRange = Tracks::normalSongs;
    // Alarm volume
    uint8_t normalVol = PLAYER_VOLUME;
    // TODO: add customization for snooze alarm (profile, time until going off, volume)

  private:
    RTC_DS3231 &_rtc;
    Timekeeper &_tk;
    DFRobotDFPlayerMini &_player;

    bool _ringing;
    AlarmTime _alarm;

    // Timers
    unsigned long _alarmMillis;

    // Callbacks
    Callback _onAlarmEvent;

    // Private helpers
    bool _isRTCAlarmEnabled();
    bool _fiveMinutesPassed();
};