#pragma once
#include "Vault.h"
#include <stdint.h>
#include <utility>

// TODO: Make namespace-based for better grouping

// config.h
// All global constants, settings, etc defined here

// ========== HARDWARE CONFIGURATION ==========

// for DS3231 control register access (writing alarms)
#define DS3231_ADDRESS 0x68
#define DS3231_CONTROL 0x0E

// RFID-RC522 module pins
constexpr uint8_t RFID_CS_PIN = 5;    // SDA/SS pin for RFID
constexpr uint8_t RFID_RST_PIN = 25;  // Reset pin for RFID
constexpr uint8_t RFID_MISO_PIN = 19; // MISO pin for RFID

// Control button pins
constexpr uint8_t BUTTON_1_PIN = 14; // top left
constexpr uint8_t BUTTON_2_PIN = 27; // top right
constexpr uint8_t BUTTON_3_PIN = 33; // bottom left
constexpr uint8_t BUTTON_4_PIN = 32; // bottom right

// PWM channel and frequency settings
static const uint8_t PWM_CHANNEL = 0;       // PWM channel (0-15 available)
static const uint32_t PWM_FREQUENCY = 5000; // 5kHz PWM frequency
static const uint8_t PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)

// TFT LED brightness control
constexpr uint8_t TFT_LED_PIN = 26;       // PWM pin for TFT backlight control
constexpr uint8_t PHOTORESISTOR_PIN = 34; // ADC pin for ambient light sensor

// For a list of all hardware pin connections refer to /README.md

// ========== DISPLAY CONFIGURATION ==========

// Systemwide default colors
constexpr uint16_t BACKGROUND_COLOR = 0x0000; // Black
constexpr uint16_t TEXT_COLOR = 0xFFFF; // White

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
constexpr uint32_t FADE_STEP_DELAY = 5;            // Milliseconds between fade steps (smaller = faster fade)
constexpr uint8_t BRIGHTNESS_CHANGE_THRESHOLD = 20; // Minimum change in brightness needed to trigger fade
                                                    // Current calibration for 4.7kΩ pulldown resistor - tuned for room lighting ~1100 = 90%+ brightness


                                                    /*
========== MicroSD Card Contents ==========
Root:
    -- Buzzer --
    0001: Three square tone 1000hz

    -- Soft(er) wakeup songs --
    0002: Never Gonna Give You Up - Rick Astley
    0003: Fortunate Son - Creedence Clearwater Revival
    0004: Mii Channel Theme But Horrendous
    0005: Circle of Life - The Lion King
    0006: Thneedville - The Lorax
    0007: Coconut Mall but in 6/8 Time SIgnature
    0008: Smooth Criminal but Every Other Beat is Missing
    0009: The Coconut Song

    -- Hard wakeup songs --
    0010: GOOOOOOD MORNING VIETNAM
    0011: BOTW Guardian Theme
    0012: SCOTLAND FOREVER
*/

namespace Tracks
{
    // Track type ranges (ex: normal songs is spanning track 2 to track 9)
    constexpr std::pair<uint8_t, uint8_t> buzzers = {1, 1};
    constexpr std::pair<uint8_t, uint8_t> normalSongs {2, 9};
    constexpr std::pair<uint8_t, uint8_t> loudSongs {10, 12};
    constexpr std::pair<uint8_t, uint8_t> allSongs {1, 12};

    constexpr uint8_t totalTrackCount = 12;
}

//!!! Going higher than 25 risks speaker damage !!!
constexpr uint8_t PLAYER_VOLUME = 20; // min 0, max 30


// ---------- Non-volatile settings ----------
// can be changed during runtime and persist between power cycles.
// Includes two functions to load/save variables to/from flash

/*This comment box contains all sensitive info constants which have been ommited
from the repository. These constants still need to be filled in or the program
won't work. References are in vault.h. Stay safe online!
// ========== NETWORK CONSTANTS ==========
WiFi and Clock sync settings
constexpr const char *WIFI_SSID = "ssid";
constexpr const char *WIFI_PASS = "password";
constexpr const char *TIME_ZONE = "UTC0"; //In POSIX time zone format

// Weather API settings (API key from OpenWeatherMap)
constexpr const char *WEATHER_API_KEY = "API_key";
constexpr const char *WEATHER_CITY = "City,ST";
constexpr const char *WEATHER_COUNTRY = "US";


// ========== RFID CONSTANTS ==========
// Master NFC tag UID
constexpr const char *ALARM_CARD_UID = "UID_here";
*/