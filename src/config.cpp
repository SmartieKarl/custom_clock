
#include <Preferences.h>
#include "config.h"

//Global volatile settings
uint8_t plrVolume = 20; // Default volume




//Global non-volatile settings
UserSettings uSet = {
	4 // almSong default: track 4
};

static Preferences prefs;

//Get settings from flash/EEPROM
void loadUserSettings() {
	prefs.begin("usercfg", true); // read-only
    //alarm song
	uSet.alarmSong = prefs.getUChar("alarmSong", uSet.alarmSong);

	prefs.end();
}

//Save settings to flash/EEPROM
void saveUserSettings() {
	prefs.begin("usercfg", false); // read-write
    //alarm song
	prefs.putUChar("alarmSong", uSet.alarmSong);

	prefs.end();
}
