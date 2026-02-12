#include "settings.h"
#include <Preferences.h>


//Global non-volatile settings
UserSettings uSet = {
	4, //almSong default
};

//Get settings from flash/EEPROM
void UserSettings::load() {
    Preferences prefs;
    prefs.begin("user", true); //read
    size_t size = prefs.getBytes("settings", this, sizeof(UserSettings));
    prefs.end();
}

//Save settings to flash/EEPROM
void UserSettings::save() {
	Preferences prefs;
    prefs.begin("user", false); //read/write
    prefs.putBytes("settings", this, sizeof(UserSettings));
    prefs.end();
}