#pragma once
#include <stdint.h>

//Brightness control module
//Handles TFT backlight brightness based on ambient light level

void initializeBrightness();
void setBrightness(uint8_t level);
void setTargetBrightness(uint8_t level);
void updateBrightnessFade();
uint8_t getBrightness();
uint8_t getTargetBrightness();
bool isBrightnessFading();
void updateAmbientBrightness();
uint16_t readLightSensor();
uint8_t calculateAmbientBrightness();