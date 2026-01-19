#include "settings_menu.h"
#include "settings.h"
#include "display.h"
#include "config.h"
#include "rtc.h"
#include "network.h"
#include "brightness.h"
#include "player.h"
#include <stdio.h>

// Extended UserSettings structure (update settings.h accordingly)
struct ExtendedSettings {
    uint8_t alarmSong;
    uint8_t volume;
    bool use24HourFormat;
    uint8_t brightnessMode;  // 0=auto, 1=manual
    uint8_t manualBrightness;
    bool alarmVibrate;
    uint8_t snoozeMinutes;
    bool weatherEnabled;
} extSettings;

// Selection options
const char* brightnessOptions[] = {"Auto", "Manual"};
const char* formatOptions[] = {"12-Hour", "24-Hour"};
const char* boolOptions[] = {"Off", "On"};

// Menu structure definitions
MenuItem mainMenuItems[] = {
    {"Alarm Settings", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, nullptr},
    {"Volume", SettingType::NUMERIC, &extSettings.volume, 0, 30, nullptr, 0, nullptr},
    {"Display", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, nullptr},
    {"Time Format", SettingType::SELECTION, &extSettings.use24HourFormat, 0, 1, formatOptions, 2, nullptr},
    {"Sync Time", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, MenuActions::syncTime},
    {"System Info", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, MenuActions::viewSystemInfo}
};

MenuItem alarmMenuItems[] = {
    {"Alarm Time", SettingType::TIME, nullptr, 0, 0, nullptr, 0, nullptr},
    {"Alarm Song", SettingType::NUMERIC, &extSettings.alarmSong, 1, 10, nullptr, 0, nullptr},
    {"Snooze Time", SettingType::NUMERIC, &extSettings.snoozeMinutes, 5, 30, nullptr, 0, nullptr},
    {"Test Alarm", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, MenuActions::testAlarm},
    {"Back", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, nullptr}
};

MenuItem displayMenuItems[] = {
    {"Brightness", SettingType::SELECTION, &extSettings.brightnessMode, 0, 1, brightnessOptions, 2, nullptr},
    {"Manual Level", SettingType::NUMERIC, &extSettings.manualBrightness, 10, 255, nullptr, 0, nullptr},
    {"Calibrate", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, MenuActions::calibrateBrightness},
    {"Back", SettingType::ACTION, nullptr, 0, 0, nullptr, 0, nullptr}
};

MenuPage menuPages[] = {
    {"Settings", mainMenuItems, 6, SettingsMenuState::MAIN_MENU},
    {"Alarm", alarmMenuItems, 5, SettingsMenuState::MAIN_MENU},
    {"Display", displayMenuItems, 4, SettingsMenuState::MAIN_MENU}
};

SettingsMenu settingsMenu;

SettingsMenu::SettingsMenu() 
    : currentState(SettingsMenuState::MAIN_MENU)
    , selectedIndex(0)
    , active(false)
    , editingValue(false) 
{
}

void SettingsMenu::enter() {
    active = true;
    selectedIndex = 0;
    editingValue = false;
    draw();
}

void SettingsMenu::exit() {
    active = false;
    uSet.save(); // Save all settings on exit
}

void SettingsMenu::navigateUp() {
    if (editingValue) return;
    
    MenuPage* page = getCurrentPage();
    if (selectedIndex > 0) {
        selectedIndex--;
    } else {
        selectedIndex = page->itemCount - 1; // Wrap to bottom
    }
    draw();
}

void SettingsMenu::navigateDown() {
    if (editingValue) return;
    
    MenuPage* page = getCurrentPage();
    if (selectedIndex < page->itemCount - 1) {
        selectedIndex++;
    } else {
        selectedIndex = 0; // Wrap to top
    }
    draw();
}

void SettingsMenu::navigateBack() {
    if (editingValue) {
        editingValue = false;
        draw();
        return;
    }
    
    MenuPage* page = getCurrentPage();
    if (page->parentMenu == currentState) {
        // Already at main menu, exit
        exit();
    } else {
        currentState = page->parentMenu;
        selectedIndex = 0;
        draw();
    }
}

void SettingsMenu::selectCurrent() {
    MenuPage* page = getCurrentPage();
    MenuItem& item = page->items[selectedIndex];
    
    switch (item.type) {
        case SettingType::ACTION:
            if (item.actionFunc) {
                item.actionFunc();
            } else {
                // Navigate to submenu based on label
                if (strcmp(item.label, "Alarm Settings") == 0) {
                    currentState = SettingsMenuState::ALARM_SETTINGS;
                    selectedIndex = 0;
                } else if (strcmp(item.label, "Display") == 0) {
                    currentState = SettingsMenuState::DISPLAY_SETTINGS;
                    selectedIndex = 0;
                } else if (strcmp(item.label, "Back") == 0) {
                    navigateBack();
                    return;
                }
            }
            break;
            
        case SettingType::TOGGLE:
        case SettingType::NUMERIC:
        case SettingType::SELECTION:
        case SettingType::TIME:
            editingValue = !editingValue;
            break;
    }
    
    draw();
}

void SettingsMenu::incrementValue() {
    if (!editingValue) return;
    
    MenuPage* page = getCurrentPage();
    MenuItem& item = page->items[selectedIndex];
    applyValueChange(item, 1);
    draw();
}

void SettingsMenu::decrementValue() {
    if (!editingValue) return;
    
    MenuPage* page = getCurrentPage();
    MenuItem& item = page->items[selectedIndex];
    applyValueChange(item, -1);
    draw();
}

MenuPage* SettingsMenu::getCurrentPage() {
    switch (currentState) {
        case SettingsMenuState::MAIN_MENU:
            return &menuPages[0];
        case SettingsMenuState::ALARM_SETTINGS:
            return &menuPages[1];
        case SettingsMenuState::DISPLAY_SETTINGS:
            return &menuPages[2];
        default:
            return &menuPages[0];
    }
}

void SettingsMenu::draw() {
    tft.fillScreen(BACKGROUND_COLOR);
    
    MenuPage* page = getCurrentPage();
    
    // Draw title
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.setTextColor(TFT_CYAN, BACKGROUND_COLOR);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(page->title, tft.width() / 2, 10);
    
    // Draw menu items
    uint8_t yPos = 50;
    for (uint8_t i = 0; i < page->itemCount; i++) {
        drawMenuItem(i, page->items[i], i == selectedIndex);
        yPos += 35;
    }
    
    // Draw button hints
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
    tft.setTextDatum(BL_DATUM);
    
    if (editingValue) {
        tft.drawString("▲ +", 10, tft.height() - 5);
        tft.drawString("▼ -", tft.width() - 40, tft.height() - 5);
    } else {
        tft.drawString("▲ Up", 10, tft.height() - 5);
        tft.drawString("▼ Down", tft.width() - 60, tft.height() - 5);
    }
    tft.setTextDatum(BR_DATUM);
    tft.drawString(editingValue ? "✓ Done" : "→ Select", tft.width() - 10, tft.height() - 5);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("← Back", tft.width() / 2 - 30, tft.height() - 5);
}

void SettingsMenu::drawMenuItem(uint8_t index, const MenuItem& item, bool selected) {
    uint16_t bgColor = selected ? TFT_DARKGREY : BACKGROUND_COLOR;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    uint8_t yPos = 50 + (index * 35);
    
    // Draw selection background
    if (selected) {
        tft.fillRect(5, yPos - 5, tft.width() - 10, 30, bgColor);
    }
    
    // Draw label
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(textColor, bgColor);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(item.label, 15, yPos + 10);
    
    // Draw value
    char valueBuffer[32];
    const char* valueStr = getValueString(item, valueBuffer, sizeof(valueBuffer));
    
    if (valueStr && strlen(valueStr) > 0) {
        uint16_t valueColor = (selected && editingValue) ? TFT_YELLOW : textColor;
        tft.setTextColor(valueColor, bgColor);
        tft.setTextDatum(MR_DATUM);
        tft.drawString(valueStr, tft.width() - 15, yPos + 10);
    }
}

const char* SettingsMenu::getValueString(const MenuItem& item, char* buffer, size_t bufSize) {
    switch (item.type) {
        case SettingType::TOGGLE:
            snprintf(buffer, bufSize, "%s", (*(bool*)item.valuePtr) ? "On" : "Off");
            return buffer;
            
        case SettingType::NUMERIC:
            snprintf(buffer, bufSize, "%d", *(int*)item.valuePtr);
            return buffer;
            
        case SettingType::SELECTION: {
            int index = *(int*)item.valuePtr;
            if (index >= 0 && index < item.optionCount) {
                return item.options[index];
            }
            return "???";
        }
            
        case SettingType::TIME: {
            AlarmTime alarm = getAlarm();
            if (extSettings.use24HourFormat) {
                snprintf(buffer, bufSize, "%02d:%02d", alarm.hour, alarm.minute);
            } else {
                uint8_t displayHour = alarm.hour % 12;
                if (displayHour == 0) displayHour = 12;
                snprintf(buffer, bufSize, "%2d:%02d %s", 
                    displayHour, alarm.minute, alarm.hour >= 12 ? "PM" : "AM");
            }
            return buffer;
        }
            
        case SettingType::ACTION:
            return ">";
            
        default:
            return "";
    }
}

void SettingsMenu::applyValueChange(MenuItem& item, int delta) {
    switch (item.type) {
        case SettingType::TOGGLE:
            *(bool*)item.valuePtr = !*(bool*)item.valuePtr;
            break;
            
        case SettingType::NUMERIC: {
            int newValue = *(int*)item.valuePtr + delta;
            if (newValue >= item.minValue && newValue <= item.maxValue) {
                *(int*)item.valuePtr = newValue;
                
                // Apply immediate effects
                if (item.valuePtr == &extSettings.volume) {
                    // Update volume immediately
                }
                if (item.valuePtr == &extSettings.manualBrightness && 
                    extSettings.brightnessMode == 1) {
                    setBrightness(extSettings.manualBrightness);
                }
            }
            break;
        }
            
        case SettingType::SELECTION: {
            int currentIndex = *(int*)item.valuePtr;
            currentIndex += delta;
            if (currentIndex < 0) currentIndex = item.optionCount - 1;
            if (currentIndex >= item.optionCount) currentIndex = 0;
            *(int*)item.valuePtr = currentIndex;
            break;
        }
            
        case SettingType::TIME: {
            AlarmTime alarm = getAlarm();
            if (delta > 0) {
                alarm.minute += 5;
                if (alarm.minute >= 60) {
                    alarm.minute = 0;
                    alarm.hour = (alarm.hour + 1) % 24;
                }
            } else {
                if (alarm.minute < 5) {
                    alarm.minute = 55;
                    alarm.hour = (alarm.hour + 23) % 24;
                } else {
                    alarm.minute -= 5;
                }
            }
            setAlarm(alarm.hour, alarm.minute, alarm.enabled);
            break;
        }
            
        default:
            break;
    }
}

// Menu action implementations
namespace MenuActions {
    void syncTime() {
        drawCenteredString("Syncing...", TFT_YELLOW);
        bool success = syncRTCFromNTP();
        flashScreen(success ? TFT_GREEN : TFT_RED, 300);
        settingsMenu.draw();
    }
    
    void updateWeather() {
        drawCenteredString("Updating...", TFT_YELLOW);
        WeatherData weather;
        bool success = fetchWeather(weather);
        flashScreen(success ? TFT_GREEN : TFT_RED, 300);
        settingsMenu.draw();
    }
    
    void testAlarm() {
        drawCenteredString("Testing Alarm...", TFT_YELLOW);
        playTrack(extSettings.alarmSong);
        delay(3000);
        stopPlayback();
        settingsMenu.draw();
    }
    
    void resetSettings() {
        drawCenteredString("Resetting...", TFT_RED);
        // Reset to defaults
        delay(1000);
        flashScreen(TFT_GREEN, 300);
        settingsMenu.draw();
    }
    
    void calibrateBrightness() {
        tft.fillScreen(BACKGROUND_COLOR);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Adjust brightness", tft.width()/2, tft.height()/2 - 20);
        tft.drawString("Press SELECT when done", tft.width()/2, tft.height()/2 + 20);
        
        // Interactive brightness adjustment
        // (Implementation depends on your button handling)
    }
    
    void viewSystemInfo() {
        tft.fillScreen(BACKGROUND_COLOR);
        tft.setTextFont(2);
        tft.setTextSize(1);
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_WHITE);
        
        DateTime now = getNow();
        char buf[64];
        
        tft.setCursor(10, 20);
        tft.println("System Information");
        tft.println("------------------");
        snprintf(buf, sizeof(buf), "RTC Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
        tft.println(buf);
        snprintf(buf, sizeof(buf), "Free Heap: %d bytes", ESP.getFreeHeap());
        tft.println(buf);
        snprintf(buf, sizeof(buf), "Brightness: %d", getBrightness());
        tft.println(buf);
        snprintf(buf, sizeof(buf), "Light Sensor: %d", readLightSensor());
        tft.println(buf);
        
        delay(5000);
        settingsMenu.draw();
    }
}