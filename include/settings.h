#pragma once
#include <stdint.h>

//Settings module
//Houses functions for both the settings display state and saving/flashing to EEPROM


struct UserSettings {
	uint8_t alarmSong;

    void load();
    void save();
};

extern UserSettings uSet;