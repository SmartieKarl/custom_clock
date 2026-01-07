#pragma once
#include "domain.h"

//RFID module
//Controls the RFID hardware object and all related functions

bool initializeRFID();

RFIDResult rfidCheckCard();
String getCardUID();