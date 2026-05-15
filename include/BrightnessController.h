#pragma once
#include "Config.h"
#include <Arduino.h>

// BrightnessController.h
// Hardware controller that updates tft backlight pin voltage using ambient light level

namespace BrightnessConfig
{
// TFT brightness control settings
constexpr uint8_t BRIGHTNESS_MIN = 1;   // Minimum disp brightness
constexpr uint8_t BRIGHTNESS_MAX = 255; // Maximum disp brightness

// Photoresistor ambient light control
constexpr uint16_t LIGHT_SENSOR_MIN = 120;       // ADC reading for darkest environment
constexpr uint16_t LIGHT_SENSOR_MAX = 1200;      // ADC reading for bright environment
constexpr uint8_t LIGHT_HYSTERESIS = 50;         // Prevent brightness flickering
constexpr uint32_t LIGHT_UPDATE_INTERVAL = 5000; // Check light level every 5 seconds

// Brightness fade settings
constexpr uint8_t FADE_STEP_SIZE = 2;               // How much to change brightness per step (smaller = smoother)
constexpr uint32_t FADE_STEP_DELAY = 5;             // Milliseconds between fade steps (smaller = faster fade)
constexpr uint8_t BRIGHTNESS_CHANGE_THRESHOLD = 20; // Minimum change in brightness needed to trigger fade
                                                    // Current calibration for 4.7kΩ pulldown resistor - tuned for room lighting ~1100 = 90%+ brightness

// PWM channel and frequency settings
static const uint8_t PWM_CHANNEL = 0;       // PWM channel (0-15 available)
static const uint32_t PWM_FREQUENCY = 5000; // 5kHz PWM frequency
static const uint8_t PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)

} // namespace BrightnessConfig

class BrightnessController
{
  public:
    BrightnessController(uint8_t pwmPin = Pins::TFT_LED_PIN,
                         uint8_t pwmChannel = BrightnessConfig::PWM_CHANNEL,
                         uint32_t pwmFreq = BrightnessConfig::PWM_FREQUENCY,
                         uint8_t pwmRes = BrightnessConfig::PWM_RESOLUTION,
                         uint8_t photoresistorPin = Pins::PHOTORESISTOR_PIN);

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