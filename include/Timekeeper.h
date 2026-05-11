#pragma once
#include "Log.h"
#include <RTClib.h>
#include <Arduino.h>

// Timekeeper.h
// Central mutex protected time access module

class Timekeeper
{
  public:
    Timekeeper(RTC_DS3231 &rtc);

    void begin();

    void update();
    DateTime time() const;

    bool tick() const;
    bool secondTick() const;
    bool minuteTick() const;
    bool hourTick() const;
    bool dayTick() const;

  private:
    RTC_DS3231 &_rtc;

    // Time state
    DateTime _curr;
    DateTime _prev;
    mutable SemaphoreHandle_t _mtx; // Mutex safety
};