#include "settings.h"
#include <Preferences.h>
#include <Arduino.h>

// Global settings instance
UserSettings uSet;

// Namespace for preferences
static const char* PREFS_NAMESPACE = "user";
static const char* PREFS_KEY = "settings";

// Load settings from flash memory
void UserSettings::load() {
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);  // Read-only mode
    
    size_t size = prefs.getBytes(PREFS_KEY, this, sizeof(UserSettings));
    
    if (size != sizeof(UserSettings)) {
        // No valid settings found, use defaults
        Serial.println("No settings found, using defaults");
        reset();
        save();  // Save defaults for next boot
    } else if (!validate()) {
        // Settings corrupted or invalid, reset to defaults
        Serial.println("Settings corrupted, resetting to defaults");
        reset();
        save();
    } else {
        Serial.println("Settings loaded successfully");
    }
    
    prefs.end();
}

// Save settings to flash memory
void UserSettings::save() {
    if (!validate()) {
        Serial.println("ERROR: Attempted to save invalid settings!");
        return;
    }
    
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, false);  // Read-write mode
    
    size_t written = prefs.putBytes(PREFS_KEY, this, sizeof(UserSettings));
    
    if (written != sizeof(UserSettings)) {
        Serial.println("ERROR: Failed to save settings!");
    } else {
        Serial.println("Settings saved successfully");
    }
    
    prefs.end();
}

// Reset settings to defaults
void UserSettings::reset() {
    alarmSong = SettingDefaults::ALARM_SONG;
    volume = SettingDefaults::VOLUME;
    use24HourFormat = SettingDefaults::USE_24_HOUR;
    brightnessMode = SettingDefaults::BRIGHTNESS_MODE;
    manualBrightness = SettingDefaults::MANUAL_BRIGHTNESS;
    snoozeMinutes = SettingDefaults::SNOOZE_MINUTES;
    weatherEnabled = SettingDefaults::WEATHER_ENABLED;
    autoTimeSync = SettingDefaults::AUTO_TIME_SYNC;
    settingsVersion = 1;
    
    Serial.println("Settings reset to defaults");
}

// Validate settings are within acceptable ranges
bool UserSettings::validate() {
    bool valid = true;
    
    // Validate and clamp volume
    if (volume > SettingLimits::VOLUME_MAX) {
        Serial.printf("Invalid volume: %d, clamping to %d\n", 
                     volume, SettingLimits::VOLUME_MAX);
        volume = SettingLimits::VOLUME_MAX;
        valid = false;
    }
    
    // Validate and clamp alarm song
    if (alarmSong < SettingLimits::ALARM_SONG_MIN || 
        alarmSong > SettingLimits::ALARM_SONG_MAX) {
        Serial.printf("Invalid alarm song: %d, resetting to default\n", alarmSong);
        alarmSong = SettingDefaults::ALARM_SONG;
        valid = false;
    }
    
    // Validate brightness mode
    if (brightnessMode > 1) {
        Serial.printf("Invalid brightness mode: %d, resetting to auto\n", brightnessMode);
        brightnessMode = 0;
        valid = false;
    }
    
    // Validate manual brightness
    if (manualBrightness < SettingLimits::BRIGHTNESS_MIN) {
        Serial.printf("Invalid manual brightness: %d, clamping to %d\n",
                     manualBrightness, SettingLimits::BRIGHTNESS_MIN);
        manualBrightness = SettingLimits::BRIGHTNESS_MIN;
        valid = false;
    }
    if (manualBrightness > SettingLimits::BRIGHTNESS_MAX) {
        Serial.printf("Invalid manual brightness: %d, clamping to %d\n",
                     manualBrightness, SettingLimits::BRIGHTNESS_MAX);
        manualBrightness = SettingLimits::BRIGHTNESS_MAX;
        valid = false;
    }
    
    // Validate snooze time
    if (snoozeMinutes < SettingLimits::SNOOZE_MIN || 
        snoozeMinutes > SettingLimits::SNOOZE_MAX) {
        Serial.printf("Invalid snooze time: %d, resetting to default\n", snoozeMinutes);
        snoozeMinutes = SettingDefaults::SNOOZE_MINUTES;
        valid = false;
    }
    
    // Check settings version for future migrations
    if (settingsVersion != 1) {
        Serial.printf("Unknown settings version: %d\n", settingsVersion);
        // For now, accept it but log
    }
    
    return valid;
}