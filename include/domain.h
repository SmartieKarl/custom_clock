#pragma once
#include <stdint.h>
#include <arduino.h>

//structs + enums kept here

// ---- App flow ----
enum class AppMode
{
    STARTUP,
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
struct ButtonStates
{
    bool btn1;
    bool btn2;
    bool btn3;
    bool btn4;
    bool any() { return btn1 || btn2 || btn3 || btn4; };
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