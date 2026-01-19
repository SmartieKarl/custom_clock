#pragma once
#include <stdint.h>
#include <Arduino.h>

//structs + enums kept here

// ---- App flow ----
enum class AppMode
{
    BOOT,
    CLOCK,
    SETTINGS
};

// ---- Time / alarm ----
struct AlarmTime
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

// ---- Weather ----
struct WeatherData
{
    float temperature;
    float tempMin;
    float tempMax;
    int humidity;
    String description;
    String mainCondition;
    bool valid;
};

// ---- Button input ----
struct ButtonState
{
    bool btn1;
    bool btn2;
    bool btn3;
    bool btn4;
    bool any() { return btn1 || btn2 || btn3 || btn4; };
    bool singlePress() { return btn1 + btn2 + btn3 + btn4 == 1; };
};

// ---- RFID states ----
enum class RFIDEvent
{
    NONE,
    ALARM_CARD,
    UNKNOWN_CARD
};

struct RFIDResult
{
    RFIDEvent event;
    String uid;
};