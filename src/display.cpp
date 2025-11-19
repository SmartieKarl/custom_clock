#include "display.h"
#include "alarm.h" // For alarm functions
#include "config.h"

// Global display state
static TFT_eSPI *tftInstance = nullptr;
bool alarmNeedsUpdate = true; // Force initial alarm display

// Initialize display system - CLEAN BASELINE
void initDisplay(TFT_eSPI &tft)
{
    tftInstance = &tft;

    // Simple clear initialization - ready for custom programming
    tft.fillScreen(TFT_BLACK);

    // Basic initialization message
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("Display Initialized");
}

// Update date display - CLEAN MINIMAL VERSION
void updateDateDisplay(const DateTime &now)
{
    if (!tftInstance)
        return;

    static uint8_t lastDay = 255;

    // Only update if day changed
    if (now.day() != lastDay)
    {
        // Simple date display
        tftInstance->fillRect(0, 20, 200, 20, TFT_BLACK);
        tftInstance->setTextColor(TFT_WHITE);
        tftInstance->setTextSize(1);
        tftInstance->setCursor(0, 20);

        tftInstance->print(now.month());
        tftInstance->print("/");
        tftInstance->print(now.day());
        tftInstance->print("/");
        tftInstance->print(now.year());

        lastDay = now.day();
    }
}

// Update time display - CLEAN MINIMAL VERSION
void updateTimeDisplay(const DateTime &now)
{
    if (!tftInstance)
        return;

    static uint8_t lastSecond = 255;

    // Update every second
    if (now.second() != lastSecond)
    {
        // Simple time display
        tftInstance->fillRect(0, 40, 200, 30, TFT_BLACK);
        tftInstance->setTextColor(TFT_WHITE);
        tftInstance->setTextSize(2);
        tftInstance->setCursor(0, 40);

        if (now.hour() < 10)
            tftInstance->print("0");
        tftInstance->print(now.hour());
        tftInstance->print(":");
        if (now.minute() < 10)
            tftInstance->print("0");
        tftInstance->print(now.minute());
        tftInstance->print(":");
        if (now.second() < 10)
            tftInstance->print("0");
        tftInstance->print(now.second());

        lastSecond = now.second();
    }
}

// Update weather display - CLEAN MINIMAL VERSION
void updateWeatherDisplay(const WeatherData &weather)
{
    if (!tftInstance)
        return;

    // Simple weather display
    tftInstance->fillRect(0, 80, 200, 40, TFT_BLACK);
    tftInstance->setTextColor(TFT_WHITE);
    tftInstance->setTextSize(1);
    tftInstance->setCursor(0, 80);

    float tempF = weather.temperature * 9.0 / 5.0 + 32.0;
    tftInstance->print("Temp: ");
    tftInstance->print((int)tempF);
    tftInstance->println("F");

    tftInstance->setCursor(0, 90);
    tftInstance->print("Humidity: ");
    tftInstance->print(weather.humidity);
    tftInstance->println("%");

    tftInstance->setCursor(0, 100);
    tftInstance->println(weather.description);
}

// Update alarm display - CLEAN MINIMAL VERSION
void updateAlarmDisplay()
{
    if (!tftInstance)
        return;

    AlarmTime currentAlarm = getCurrentAlarmTime();

    // Simple alarm display
    tftInstance->fillRect(0, 130, 200, 20, TFT_BLACK);
    tftInstance->setTextColor(TFT_WHITE);
    tftInstance->setTextSize(1);
    tftInstance->setCursor(0, 130);

    tftInstance->print("Alarm: ");
    if (currentAlarm.hour < 10)
        tftInstance->print("0");
    tftInstance->print(currentAlarm.hour);
    tftInstance->print(":");
    if (currentAlarm.minute < 10)
        tftInstance->print("0");
    tftInstance->print(currentAlarm.minute);

    if (isAlarmTriggered())
    {
        tftInstance->print(" RINGING");
    }
    else
    {
        tftInstance->print(" SET");
    }
}

// Set alarm update flag
void setAlarmUpdateFlag(bool needsUpdate)
{
    alarmNeedsUpdate = needsUpdate;
}

// Flash the entire screen with a color for visual feedback
void flashScreen(uint16_t flashColor, int flashDuration)
{
    if (!tftInstance)
        return;

    tftInstance->fillScreen(flashColor);
    delay(flashDuration);
    tftInstance->fillScreen(TFT_BLACK);
    delay(50);
}

// ========== BRIGHTNESS CONTROL FUNCTIONS ==========
// (Merged from brightness.cpp - brightness is a display characteristic)

// PWM channel and frequency settings
static const uint8_t PWM_CHANNEL = 0;       // PWM channel (0-15 available)
static const uint32_t PWM_FREQUENCY = 5000; // 5kHz PWM frequency
static const uint8_t PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)

// Current brightness state
static uint8_t currentBrightness = BRIGHTNESS_MAX; // Start at max, will auto-adjust in setup
static uint16_t lastLightReading = 0;              // For hysteresis
static uint8_t targetBrightness = BRIGHTNESS_MAX;  // Target brightness for fading
static unsigned long lastFadeUpdate = 0;           // Timing for fade steps

// Initialize PWM for brightness control
void initializeBrightness()
{
    // Configure PWM channel
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);

    // Attach pin to PWM channel
    ledcAttachPin(TFT_LED_PIN, PWM_CHANNEL);

    // Initialize photoresistor pin (ADC)
    pinMode(PHOTORESISTOR_PIN, INPUT);

    // Start at 100% brightness for setup - loop() will adjust to ambient automatically
    setBrightness(BRIGHTNESS_MAX);

    // Read initial light for debugging
    uint16_t initialLight = readLightSensor();
}

// Set brightness level (0-255) - immediate change
void setBrightness(uint8_t level)
{
    // Ensure minimum brightness (never completely off)
    if (level < BRIGHTNESS_MIN && level > 0)
    {
        level = BRIGHTNESS_MIN;
    }

    currentBrightness = level;
    targetBrightness = level; // Also update target to prevent conflicts
    ledcWrite(PWM_CHANNEL, level);
}

// Set target brightness for smooth fading
void setTargetBrightness(uint8_t level)
{
    // Ensure minimum brightness (never completely off)
    if (level < BRIGHTNESS_MIN && level > 0)
    {
        level = BRIGHTNESS_MIN;
    }

    targetBrightness = level;
}

// Update fade progress - call this regularly in main loop
void updateBrightnessFade()
{
    unsigned long currentTime = millis();

    // Only update fade if enough time has passed and we need to change
    if (currentTime - lastFadeUpdate >= FADE_STEP_DELAY && currentBrightness != targetBrightness)
    {

        // Determine fade direction and step size
        if (currentBrightness < targetBrightness)
        {
            // Fade up
            uint8_t difference = targetBrightness - currentBrightness;
            uint8_t step = min(FADE_STEP_SIZE, difference);
            currentBrightness += step;
        }
        else
        {
            // Fade down
            uint8_t difference = currentBrightness - targetBrightness;
            uint8_t step = min(FADE_STEP_SIZE, difference);
            currentBrightness -= step;
        }

        // Apply the new brightness
        ledcWrite(PWM_CHANNEL, currentBrightness);
        lastFadeUpdate = currentTime;
    }
}

// Get current brightness level
uint8_t getBrightness()
{
    return currentBrightness;
}

// Get target brightness level
uint8_t getTargetBrightness()
{
    return targetBrightness;
}

// Check if brightness is currently fading
bool isBrightnessFading()
{
    return currentBrightness != targetBrightness;
}

// Read photoresistor value (0-4095 on ESP32)
uint16_t readLightSensor()
{
    return analogRead(PHOTORESISTOR_PIN);
}

// Calculate brightness based on ambient light
uint8_t calculateAmbientBrightness()
{
    uint16_t lightLevel = readLightSensor();

    // Apply hysteresis to prevent flickering
    if (abs(lightLevel - lastLightReading) < LIGHT_HYSTERESIS)
    {
        lightLevel = lastLightReading; // Use previous reading if change is small
    }
    else
    {
        lastLightReading = lightLevel;
    }

    // Map light sensor reading to brightness level
    // Clamp to min/max range first
    if (lightLevel < LIGHT_SENSOR_MIN)
        lightLevel = LIGHT_SENSOR_MIN;
    if (lightLevel > LIGHT_SENSOR_MAX)
        lightLevel = LIGHT_SENSOR_MAX;

    // Map to brightness range (inverted: less light = less brightness)
    uint8_t brightness = map(lightLevel, LIGHT_SENSOR_MIN, LIGHT_SENSOR_MAX,
                             BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    return brightness;
}

// Update brightness based on ambient light with smooth fading
void updateAmbientBrightness()
{
    static unsigned long lastUpdate = 0;
    unsigned long currentTime = millis();

    // Always update the fade progress
    updateBrightnessFade();

    // Check ambient light every few seconds
    if (currentTime - lastUpdate >= LIGHT_UPDATE_INTERVAL)
    {
        uint16_t rawReading = readLightSensor();
        uint8_t newTargetBrightness = calculateAmbientBrightness();

        // Only change if significantly different to avoid constant adjustments
        if (abs(targetBrightness - newTargetBrightness) > BRIGHTNESS_CHANGE_THRESHOLD)
        {
            setTargetBrightness(newTargetBrightness);
        }

        lastUpdate = currentTime;
    }
}
