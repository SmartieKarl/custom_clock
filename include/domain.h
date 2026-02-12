#pragma once
#include <stdint.h>
#include <Arduino.h>

// domain.h
//Structs + enums that more than one file needs kept here

// ========== APPLICATION STATE ==========
enum class AppMode
{
    BOOT,
    CLOCK,
    SETTINGS
};

// ========== TIME & ALARM ==========
struct AlarmTime
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

// ========== WEATHER ==========
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

// ========== INPUT ==========
struct ButtonState
{
    bool btn1;
    bool btn2;
    bool btn3;
    bool btn4;
    bool any() { return btn1 || btn2 || btn3 || btn4; };
    bool singlePress() { return btn1 + btn2 + btn3 + btn4 == 1; };
};

// ========== RFID ==========
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