#pragma once
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

//DFPlayer module
//Controls the DFPlayer hardware object and all related functions


extern DFRobotDFPlayerMini player;
extern HardwareSerial mySoftwareSerial;

bool initializeDFPlayer(DFRobotDFPlayerMini &player, HardwareSerial &serial);

//Alarm functions
void stopAlarm(DFRobotDFPlayerMini &player);