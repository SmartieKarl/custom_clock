#pragma once
#include "domain.h"
#include <stdint.h>

// Settings menu navigation and management
// Provides a hierarchical menu system for user configuration

// Menu states for navigation
enum class SettingsMenuState {
    MAIN_MENU,
    ALARM_SETTINGS,
    VOLUME_SETTINGS,
    DISPLAY_SETTINGS,
    TIME_SETTINGS,
    NETWORK_SETTINGS
};

// Individual setting types
enum class SettingType {
    TOGGLE,      // On/Off
    NUMERIC,     // Number with min/max
    SELECTION,   // Choose from list
    TIME,        // Hour:Minute
    ACTION       // Execute function
};

// Setting item definition
struct MenuItem {
    const char* label;
    SettingType type;
    void* valuePtr;           // Pointer to the actual setting value
    int minValue;             // For numeric types
    int maxValue;             // For numeric types
    const char** options;     // For selection types
    int optionCount;          // Number of options
    void (*actionFunc)();     // For action types
};

// Menu page definition
struct MenuPage {
    const char* title;
    MenuItem* items;
    uint8_t itemCount;
    SettingsMenuState parentMenu;  // For back navigation
};

// Settings menu manager class
class SettingsMenu {
public:
    SettingsMenu();
    
    // Navigation
    void enter();
    void exit();
    void navigateUp();
    void navigateDown();
    void navigateBack();
    void selectCurrent();
    
    // Value modification
    void incrementValue();
    void decrementValue();
    
    // Rendering
    void draw();
    void update();
    
    // State queries
    SettingsMenuState getCurrentState() const { return currentState; }
    bool isActive() const { return active; }
    
private:
    SettingsMenuState currentState;
    uint8_t selectedIndex;
    bool active;
    bool editingValue;
    
    // Menu structure
    MenuPage* getCurrentPage();
    void drawMenuItem(uint8_t index, const MenuItem& item, bool selected);
    void drawValueEditor(const MenuItem& item);
    
    // Helper functions
    const char* getValueString(const MenuItem& item, char* buffer, size_t bufSize);
    void applyValueChange(MenuItem& item, int delta);
};

// Global settings menu instance
extern SettingsMenu settingsMenu;

// Menu action callbacks
namespace MenuActions {
    void syncTime();
    void updateWeather();
    void testAlarm();
    void resetSettings();
    void calibrateBrightness();
    void viewSystemInfo();
}