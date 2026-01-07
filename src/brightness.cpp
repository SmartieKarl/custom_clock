#include <Arduino.h>
#include "brightness.h"
#include "config.h"
//current brightness state
static uint8_t currentBrightness = BRIGHTNESS_MAX; // Start at max, will auto-adjust in setup
static uint16_t lastLightReading = 0;              // For hysteresis
static uint8_t targetBrightness = BRIGHTNESS_MAX;  // Target brightness for fading
static unsigned long lastFadeUpdate = 0;           // Timing for fade steps

//initialize PWM for brightness control
void initializeBrightness()
{
    //configure PWM channel
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);

    //attach pin to PWM channel
    ledcAttachPin(TFT_LED_PIN, PWM_CHANNEL);

    //initialize photoresistor pin (ADC)
    pinMode(PHOTORESISTOR_PIN, INPUT);

    //start at 100% brightness for setup - loop() will adjust to ambient automatically
    setBrightness(BRIGHTNESS_MAX);

    //read initial light for debugging
    uint16_t initialLight = readLightSensor();
}

//set brightness level (0-255) - immediate change
void setBrightness(uint8_t level)
{
    if (level < BRIGHTNESS_MIN && level > 0)
    {
        level = BRIGHTNESS_MIN;
    }

    currentBrightness = level;
    targetBrightness = level; //also update target to prevent conflicts
    ledcWrite(PWM_CHANNEL, level);
}

//set target brightness for smooth fading
void setTargetBrightness(uint8_t level)
{
    if (level < BRIGHTNESS_MIN && level > 0)
    {
        level = BRIGHTNESS_MIN;
    }

    targetBrightness = level;
}

//update fade progress
void updateBrightnessFade()
{
    unsigned long currentTime = millis();

    //inly update fade if enough time has passed and we need to change
    if (currentTime - lastFadeUpdate >= FADE_STEP_DELAY && currentBrightness != targetBrightness)
    {

        //determine fade direction and step size
        if (currentBrightness < targetBrightness)
        {
            //fade up
            uint8_t difference = targetBrightness - currentBrightness;
            uint8_t step = min(FADE_STEP_SIZE, difference);
            currentBrightness += step;
        }
        else
        {
            //fade down
            uint8_t difference = currentBrightness - targetBrightness;
            uint8_t step = min(FADE_STEP_SIZE, difference);
            currentBrightness -= step;
        }

        //apply the new brightness
        ledcWrite(PWM_CHANNEL, currentBrightness);
        lastFadeUpdate = currentTime;
    }
}

//get current brightness level
uint8_t getBrightness()
{
    return currentBrightness;
}

//get target brightness level
uint8_t getTargetBrightness()
{
    return targetBrightness;
}

//check if brightness is currently fading
bool isBrightnessFading()
{
    return currentBrightness != targetBrightness;
}

//read photoresistor value (0-4095 on ESP32)
uint16_t readLightSensor()
{
    return analogRead(PHOTORESISTOR_PIN);
}

//calculate brightness based on ambient light
uint8_t calculateAmbientBrightness()
{
    uint16_t lightLevel = readLightSensor();

    //apply hysteresis to prevent flickering
    if (abs(lightLevel - lastLightReading) < LIGHT_HYSTERESIS)
    {
        lightLevel = lastLightReading; //use previous reading if change is small
    }
    else
    {
        lastLightReading = lightLevel;
    }

    //map light sensor reading to brightness level
    //clamp to min/max range first
    if (lightLevel < LIGHT_SENSOR_MIN)
        lightLevel = LIGHT_SENSOR_MIN;
    if (lightLevel > LIGHT_SENSOR_MAX)
        lightLevel = LIGHT_SENSOR_MAX;

    //map to brightness range (inverted: less light = less brightness)
    uint8_t brightness = map(lightLevel, LIGHT_SENSOR_MIN, LIGHT_SENSOR_MAX,
                             BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    return brightness;
}

//update brightness based on ambient light with smooth fading
void updateAmbientBrightness()
{
    static unsigned long lastUpdate = 0;
    unsigned long currentTime = millis();

    //always update the fade progress
    updateBrightnessFade();

    //check ambient light every few seconds
    if (currentTime - lastUpdate >= LIGHT_UPDATE_INTERVAL)
    {
        uint16_t rawReading = readLightSensor();
        uint8_t newTargetBrightness = calculateAmbientBrightness();

        //only change if significantly different to avoid constant adjustments
        if (abs(targetBrightness - newTargetBrightness) > BRIGHTNESS_CHANGE_THRESHOLD)
        {
            setTargetBrightness(newTargetBrightness);
        }

        lastUpdate = currentTime;
    }
}