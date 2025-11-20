#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <RTClib.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

// network.h
// Handles all functions related to wireless connectivity



// Weather data structure
struct WeatherData
{
    float temperature; // in Fahrenheit
    float tempMin;
    float tempMax;
    int humidity;
    String description;
    String mainCondition; // e.g., "Clear", "Clouds", "Rain"
    bool valid;
};

// Network functions

// Connects to WiFi
bool connectWiFi(const char *ssid, const char *pass, unsigned long timeoutMs = 15000);
// Disconnects from WiFi
void disconnectWiFi();
// Syncs system time from NTP, writes it to the DS3231
bool syncRTCFromNTP();
// Fetch weather data from OpenWeatherMap API
bool fetchWeather(WeatherData &weather);