#pragma once
#include "vault.h"
#include <stdint.h>
#include <TFT_eSPI.h>

// config.h
// All global constants, settings, etc defined here

//for DS3231 control register access (writing alarms)
#define DS3231_ADDRESS 0x68
#define DS3231_CONTROL 0x0E


// ========== HARDWARE PINS ==========
// RFID-RC522 module pins
constexpr uint8_t RFID_CS_PIN = 5;    // SDA/SS pin for RFID
constexpr uint8_t RFID_RST_PIN = 25;  // Reset pin for RFID
constexpr uint8_t RFID_MISO_PIN = 19; // MISO pin for RFID

//Control button pins
constexpr uint8_t BUTTON_1_PIN = 14; // top left
constexpr uint8_t BUTTON_2_PIN = 27; // top right
constexpr uint8_t BUTTON_3_PIN = 33; // bottom left
constexpr uint8_t BUTTON_4_PIN = 32; // bottom right

// MOSI (23) and SCK (18) are shared with TFT display
// TFT display pins defined in User_Setup.h (/include/User_Setup.h)

// TFT LED brightness control
constexpr uint8_t TFT_LED_PIN = 26;       // PWM pin for TFT backlight control
constexpr uint8_t PHOTORESISTOR_PIN = 34; // ADC pin for ambient light sensor

//For a list of all hardware pin connections refer to /README.md



// ========== SYSTEM TIMING CONSTANTS ==========
constexpr uint16_t LOOP_DELAY = 50; // Loop delay so things don't explode (DON'T SET TO 0)



// ========== DISPLAY CONSTANTS ==========

//Systemwide default colors
constexpr uint16_t BACKGROUND_COLOR = TFT_BLACK;
constexpr uint16_t TEXT_COLOR = TFT_WHITE;

// PWM channel and frequency settings
static const uint8_t PWM_CHANNEL = 0;       // PWM channel (0-15 available)
static const uint32_t PWM_FREQUENCY = 5000; // 5kHz PWM frequency
static const uint8_t PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)

// TFT brightness control settings
constexpr uint8_t BRIGHTNESS_MIN = 10;  // Minimum disp brightness (4%)
constexpr uint8_t BRIGHTNESS_MAX = 255; // Maximum disp brightness (100%)

// Photoresistor ambient light control
constexpr uint16_t LIGHT_SENSOR_MIN = 120;       // ADC reading for darkest environment
constexpr uint16_t LIGHT_SENSOR_MAX = 1200;      // ADC reading for bright environment
constexpr uint8_t LIGHT_HYSTERESIS = 50;         // Prevent brightness flickering
constexpr uint32_t LIGHT_UPDATE_INTERVAL = 5000; // Check light level every 5 seconds

// Brightness fade settings
constexpr uint8_t FADE_STEP_SIZE = 2;               // How much to change brightness per step (smaller = smoother)
constexpr uint32_t FADE_STEP_DELAY = 25;            // Milliseconds between fade steps (smaller = faster fade)
constexpr uint8_t BRIGHTNESS_CHANGE_THRESHOLD = 15; // Minimum change in brightness needed to trigger fade
// Current calibration for 4.7kΩ pulldown resistor - tuned for room lighting ~1100 = 90%+ brightness



// ========== USER SETTINGS ==========

// ---------- Non-volatile settings ----------
//can be changed during runtime and persist between power cycles.
//Includes two functions to load/save variables to/from flash




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