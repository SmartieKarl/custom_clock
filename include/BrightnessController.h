#pragma once
#include "Config.h"
#include <Arduino.h>

// BrightnessController.h
// Hardware controller that updates tft backlight pin voltage
// using ambient light level
class BrightnessController
{
  public:
    BrightnessController(uint8_t pwmPin = TFT_LED_PIN,
                         uint8_t pwmChannel = PWM_CHANNEL,
                         uint32_t pwmFreq = PWM_FREQUENCY,
                         uint8_t pwmRes = PWM_RESOLUTION,
                         uint8_t photoresistorPin = PHOTORESISTOR_PIN);

    void begin();

    void setBrightness(uint8_t level);
    void setTargetBrightness(uint8_t level);

    void updateFade();
    void updateAmbient();

    uint8_t getBrightness() const;
    uint8_t getTargetBrightness() const;
    bool isFading() const;

  private:
    uint16_t readLightSensor() const;
    uint8_t calculateAmbientBrightness();

  private:
    uint8_t _pwmPin;
    uint8_t _pwmChannel;
    uint32_t _pwmFreq;
    uint8_t _pwmRes;
    uint8_t _photoresistorPin;

    uint8_t _currentBrightness;
    uint8_t _targetBrightness;

    uint16_t _lastLightReading;
    unsigned long _lastFadeUpdate;
    unsigned long _lastAmbientUpdate;
};