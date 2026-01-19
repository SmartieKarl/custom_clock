#pragma once
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

//DFPlayer module
//Controls the DFPlayer hardware object and all related functions


extern DFRobotDFPlayerMini player;
extern HardwareSerial mySoftwareSerial;

bool initializeDFPlayer(HardwareSerial &serial);

//Alarm functions
void stopPlayback();

void playTrack(int trackNumber);

void loopTrack(int trackNumber);

void playAllOnShuffle();