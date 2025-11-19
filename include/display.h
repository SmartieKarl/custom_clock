#pragma once

#include "config.h"  // For brightness constants
#include "network.h" // For WeatherData struct
#include <Arduino.h>
#include <RTClib.h>
#include <TFT_eSPI.h>

// Display module for alarm clock project
// Sprite-based double-buffered display with ergonomic layout:
// - Date: Top left corner
// - Weather: Top right corner (simplified)
// - Time: Large, centered
// - Alarm: Bottom, simplified

// Global variables (declared here, defined in display.cpp)
extern bool alarmNeedsUpdate;

// Display initialization functions
void initDisplay(TFT_eSPI &tft);

// Display update functions
void updateDateDisplay(const DateTime &now);           // Update date in top left
void updateTimeDisplay(const DateTime &now);           // Update large centered time
void updateWeatherDisplay(const WeatherData &weather); // Update simplified weather
void updateAlarmDisplay();                             // Update simplified alarm

// Display utility functions
void setAlarmUpdateFlag(bool needsUpdate);
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