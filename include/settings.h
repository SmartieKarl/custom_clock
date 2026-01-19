#pragma once
#include <stdint.h>

// Settings module - manages persistent user preferences

// Define setting ranges and defaults
namespace SettingDefaults {
    constexpr uint8_t ALARM_SONG = 4;
    constexpr uint8_t VOLUME = 20;
    constexpr bool USE_24_HOUR = false;
    constexpr uint8_t BRIGHTNESS_MODE = 0;  // 0=auto, 1=manual
    constexpr uint8_t MANUAL_BRIGHTNESS = 200;
    constexpr uint8_t SNOOZE_MINUTES = 10;
    constexpr bool WEATHER_ENABLED = true;
    constexpr bool AUTO_TIME_SYNC = true;
}

namespace SettingLimits {
    constexpr uint8_t VOLUME_MIN = 0;
    constexpr uint8_t VOLUME_MAX = 30;
    constexpr uint8_t ALARM_SONG_MIN = 1;
    constexpr uint8_t ALARM_SONG_MAX = 10;
    constexpr uint8_t SNOOZE_MIN = 5;
    constexpr uint8_t SNOOZE_MAX = 30;
    constexpr uint8_t BRIGHTNESS_MIN = 10;
    constexpr uint8_t BRIGHTNESS_MAX = 255;
}

// User settings structure - persisted to flash memory
struct UserSettings {
    // Audio settings
    uint8_t alarmSong;
    uint8_t volume;
    
    // Display settings
    bool use24HourFormat;
    uint8_t brightnessMode;      // 0=auto, 1=manual
    uint8_t manualBrightness;
    
    // Alarm settings
    uint8_t snoozeMinutes;
    
    // Network settings
    bool weatherEnabled;
    bool autoTimeSync;
    
    // Settings version for migration
    uint8_t settingsVersion;
    
    // Methods
    void load();
    void save();
    void reset();
    bool validate();
    
    // Constructor with defaults
    UserSettings() 
        : alarmSong(SettingDefaults::ALARM_SONG)
        , volume(SettingDefaults::VOLUME)
        , use24HourFormat(SettingDefaults::USE_24_HOUR)
        , brightnessMode(SettingDefaults::BRIGHTNESS_MODE)
        , manualBrightness(SettingDefaults::MANUAL_BRIGHTNESS)
        , snoozeMinutes(SettingDefaults::SNOOZE_MINUTES)
        , weatherEnabled(SettingDefaults::WEATHER_ENABLED)
        , autoTimeSync(SettingDefaults::AUTO_TIME_SYNC)
        , settingsVersion(1)
    {}
};

// Global settings instance
extern UserSettings uSet;