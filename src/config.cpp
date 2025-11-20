
#include <Preferences.h>
#include "config.h"

//Global volatile settings
uint8_t plrVolume = 20; // Default volume




//Global non-volatile settings
UserSettings userSettings = {
	4 // alarmSong default
};

static Preferences prefs;

//Get settings from flash/EEPROM
void loadUserSettings() {
	prefs.begin("usercfg", true); // read-only
	userSettings.alarmSong = prefs.getUChar("alarmSong", userSettings.alarmSong);
	// Add more fields here as you add them to the struct
	prefs.end();
}

//Save settings to flash/EEPROM
void saveUserSettings() {
	prefs.begin("usercfg", false); // read-write
	prefs.putUChar("alarmSong", userSettings.alarmSong);
	// Add more fields here as you add them to the struct
	prefs.end();
}
