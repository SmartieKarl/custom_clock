#pragma once

#include "config.h"
#include "network.h"
#include <Arduino.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Display module
// Handles all display write functions and brightness control



// Display area update functions
void updateDateDisplay(const DateTime &now);
void updateTimeDisplay(const DateTime &now);
void updateWeatherDisplay(const WeatherData &weather);
void updateAlarmDisplay();

// Display utility functions
void flashScreen(uint16_t flashColor, int flashDuration = 150);

// ========== BRIGHTNESS CONTROL FUNCTIONS ==========

// Brightness control functions
void initializeBrightness();
void setBrightness(uint8_t level);       // Set brightness 0-255 immediately
void setTargetBrightness(uint8_t level); // Set target brightness for smooth fading
void updateBrightnessFade();             // Update fade progress (call regularly)
uint8_t getBrightness();                 // Get current brightness
uint8_t getTargetBrightness();           // Get target brightness
bool isBrightnessFading();               // Check if currently fading

// Photoresistor-based ambient light control
void updateAmbientBrightness();       // Update based on light sensor with fade
uint16_t readLightSensor();           // Read raw photoresistor value
uint8_t calculateAmbientBrightness(); // Convert light to brightness level