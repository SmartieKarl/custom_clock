#pragma once
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// DFPlayer module
// Controls the DFPlayer hardware object and all related functions

/*
========== CURRENT TRACKS ON DFPLAYER ==========
01: Soft(er) wakeup
    001: Never Gonna Give You Up - Rick Astley
    002: Fortunate Son - Creedence Clearwater Revival
    003: Mii Channel Theme But Horrendous
    004: Circle of Life - The Lion King
    005: Thneedville - The Lorax
    006: Coconut Mall but in 6/8 Time SIgnature
    007: Smooth Criminal but Every Other Beat is Missing
    008: The Coconut Song

02: Hard wakeup
    001: GOOOOOOD MORNING VIETNAM
    002: BOTW Guardian Theme
    003: SCOTLAND FOREVER
*/

//!!! Going higher than 25 risks speaker damage !!!
constexpr uint8_t PLAYER_VOLUME = 20; // min 0, max 30

extern DFRobotDFPlayerMini player;
extern HardwareSerial mySoftwareSerial;

bool initializeDFPlayer(HardwareSerial &serial);

// Alarm functions
void stopAudioPlayback();

void playTrack(int track, int folder = 1, int vol = PLAYER_VOLUME);

//does not work, will change later
void loopTrack(int track, int folder = 1, int vol = PLAYER_VOLUME);

void playAllOnShuffle();

void setVolume(int vol);

void loopPlayerFolder(int folder);