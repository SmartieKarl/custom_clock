
#include <Preferences.h>
#include "config.h"

//Global volatile settings
uint8_t plrVolume = 20; // Default volume




//Global non-volatile settings
UserSettings uSet = {
    {7, 0, true}, // almTime default: 7:00 AM, enabled
	4 // almSong default: track 4
};

static Preferences prefs;

//Get settings from flash/EEPROM
void loadUserSettings() {
	prefs.begin("usercfg", true); // read-only

    //alarm time info
    uSet.alarmTime.hour = prefs.getUChar("alarmHour", uSet.alarmTime.hour);
    uSet.alarmTime.minute = prefs.getUChar("alarmMinute", uSet.alarmTime.minute);
    uSet.alarmTime.enabled = prefs.getBool("alarmEnabled", uSet.alarmTime.enabled);
    //alarm song
	uSet.alarmSong = prefs.getUChar("alarmSong", uSet.alarmSong);

	prefs.end();
}

//Save settings to flash/EEPROM
void saveUserSettings() {
	prefs.begin("usercfg", false); // read-write

    //alarm time info
    prefs.putUChar("alarmHour", uSet.alarmTime.hour);
    prefs.putUChar("alarmMinute", uSet.alarmTime.minute);
    prefs.putBool("alarmEnabled", uSet.alarmTime.enabled);
    //alarm song
	prefs.putUChar("alarmSong", uSet.alarmSong);

	prefs.end();
}
