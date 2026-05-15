#pragma once
#include "Log.h"
#include <Arduino.h>
#include <RTClib.h> // RTC access for time sync

// NetworkManager.h
// Mutex-protected network interaction module

struct WeatherData
{
    float temperature;
    float tempMin;
    float tempMax;
    int humidity;
    char description[64];
    char mainCondition[32];
    bool valid;
};

class NetworkManager
{
  public:
    NetworkManager(RTC_DS3231 &rtc);

    void begin();

    bool startWiFiSession();
    void endWiFiSession();

    bool fetchWeather();
    WeatherData currentWeather = {0, 0, 0, 0, "", "", false};

    bool syncRTCFromNTP();

    // WiFi persistence control
    void setWiFiPersistent(bool persistent);
    bool isWiFiPersistent() const;

  private:
    uint8_t _users;
    bool _persistent;
    bool _connecting;

    // Objects
    RTC_DS3231 &_rtc;

    mutable SemaphoreHandle_t _mtx; // Mutex safety
};