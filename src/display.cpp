#include "display.h"
#include "alarm.h"
#include "config.h"

// Global display state
static TFT_eSPI *tftInstance = nullptr;

// Assign the global TFT object to the display module
void setTFTInstance(TFT_eSPI *instance) {
    tftInstance = instance;
}


// Update time display
void updateTimeDisplay(const DateTime &now)
{
    if (!tftInstance)
        return;

    tftInstance->fillRect(0, 40, 90, 16, TFT_BLACK);
    tftInstance->setTextColor(TFT_WHITE, TFT_BLACK);
    tftInstance->setTextSize(2);
    tftInstance->setCursor(2, 40);
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    tftInstance->print(buf);
}

// Update date display
void updateDateDisplay(const DateTime &now)
{
    if (!tftInstance)
        return;

    tftInstance->fillRect(0, 20, 90, 16, TFT_BLACK);
    tftInstance->setTextColor(TFT_YELLOW, TFT_BLACK);
    tftInstance->setTextSize(1);
    tftInstance->setCursor(2, 20);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", now.month(), now.day(), now.year());
    tftInstance->print(buf);
}

// Update weather display
void updateWeatherDisplay(const WeatherData &weather)
{
    if (!tftInstance)
        return;

    tftInstance->fillRect(120, 80, 120, 32, TFT_BLACK);
    tftInstance->setTextColor(TFT_CYAN, TFT_BLACK);
    tftInstance->setTextSize(1);
    tftInstance->setCursor(122, 80);
    char buf[32];
    snprintf(buf, sizeof(buf), "Temp: %dF", (int)weather.temperature);
    tftInstance->println(buf);
    tftInstance->setCursor(122, 96);
    snprintf(buf, sizeof(buf), "Humidity: %d%%", weather.humidity);
    tftInstance->println(buf);
}

// Update alarm display
void updateAlarmDisplay()
{
    if (!tftInstance)
        return;

    AlarmTime alarm = getAlarm();
    tftInstance->fillRect(0, 130, 120, 16, TFT_BLACK);
    tftInstance->setTextColor(TFT_MAGENTA, TFT_BLACK);
    tftInstance->setTextSize(1);
    tftInstance->setCursor(2, 130);
    char buf[32];
    snprintf(buf, sizeof(buf), "Alarm: %02d:%02d %s", alarm.hour, alarm.minute, isAlarmRinging() ? "RINGING" : "SET");
    tftInstance->print(buf);
}

// Flash screen with color for set duration.
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
//ChatGPT wrote this part, but
// 1) it works and
// 2) I don't know how to write this myself


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
    if (level < BRIGHTNESS_MIN && level > 0)
    {
        level = BRIGHTNESS_MIN;
    }

    targetBrightness = level;
}

// Update fade progress
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
