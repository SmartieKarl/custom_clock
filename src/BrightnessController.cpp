#include "BrightnessController.h"

using namespace BrightnessConfig;
// Constructor
BrightnessController::BrightnessController(uint8_t pwmPin,
                                           uint8_t pwmChannel,
                                           uint32_t pwmFreq,
                                           uint8_t pwmRes,
                                           uint8_t photoresistorPin)
    : _pwmPin(pwmPin), _pwmChannel(pwmChannel),
      _pwmFreq(pwmFreq), _pwmRes(pwmRes),
      _photoresistorPin(photoresistorPin),
      _currentBrightness(BRIGHTNESS_MAX),
      _targetBrightness(BRIGHTNESS_MAX),
      _lastLightReading(0),
      _lastFadeUpdate(0),
      _lastAmbientUpdate(0)
{
}

void BrightnessController::begin()
{
    ledcSetup(_pwmChannel, _pwmFreq, _pwmRes);
    ledcAttachPin(_pwmPin, _pwmChannel);
    pinMode(_photoresistorPin, INPUT);

    setBrightness(BRIGHTNESS_MAX);
    _lastLightReading = readLightSensor();
}

void BrightnessController::setBrightness(uint8_t level)
{
    if (level < BRIGHTNESS_MIN && level > 0)
        level = BRIGHTNESS_MIN;

    _currentBrightness = level;
    _targetBrightness = level;
    ledcWrite(_pwmChannel, _currentBrightness);
}

void BrightnessController::setTargetBrightness(uint8_t level)
{
    if (level < BRIGHTNESS_MIN && level > 0)
        level = BRIGHTNESS_MIN;

    _targetBrightness = level;
}

void BrightnessController::updateFade()
{
    unsigned long now = millis();
    if (now - _lastFadeUpdate < FADE_STEP_DELAY)
        return;
    if (_currentBrightness == _targetBrightness)
        return;

    if (_currentBrightness < _targetBrightness)
    {
        uint8_t step = (uint8_t)std::min((int)FADE_STEP_SIZE, (int)(_targetBrightness - _currentBrightness));
        _currentBrightness += step;
    }
    else
    {
        uint8_t step = (uint8_t)std::min((int)FADE_STEP_SIZE, (int)(_currentBrightness - _targetBrightness));
        _currentBrightness -= step;
    }

    ledcWrite(_pwmChannel, _currentBrightness);
    _lastFadeUpdate = now;
}

uint16_t BrightnessController::readLightSensor() const
{
    return analogRead(_photoresistorPin);
}

uint8_t BrightnessController::calculateAmbientBrightness()
{
    uint16_t light = readLightSensor();

    if (abs((int)light - (int)_lastLightReading) < LIGHT_HYSTERESIS)
        light = _lastLightReading;
    else
        _lastLightReading = light;

    if (light < LIGHT_SENSOR_MIN)
        light = LIGHT_SENSOR_MIN;
    if (light > LIGHT_SENSOR_MAX)
        light = LIGHT_SENSOR_MAX;

    return map(light, LIGHT_SENSOR_MIN, LIGHT_SENSOR_MAX,
               BRIGHTNESS_MIN, BRIGHTNESS_MAX);
}

void BrightnessController::updateAmbient()
{
    unsigned long now = millis();

    updateFade();

    if (now - _lastAmbientUpdate < LIGHT_UPDATE_INTERVAL)
        return;

    uint8_t newTarget = calculateAmbientBrightness();
    if (abs((int)newTarget - (int)_targetBrightness) > BRIGHTNESS_CHANGE_THRESHOLD)
        setTargetBrightness(newTarget);

    _lastAmbientUpdate = now;
}

uint8_t BrightnessController::getBrightness() const { return _currentBrightness; }
uint8_t BrightnessController::getTargetBrightness() const { return _targetBrightness; }
bool BrightnessController::isFading() const { return _currentBrightness != _targetBrightness; }