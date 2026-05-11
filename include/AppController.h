#pragma once
#include "Buttons.h"             // Buttons hardware driver
#include "RFIDHandler.h"         // RFID input abstracion layer
#include <DFRobotDFPlayerMini.h> // Audio control

#include "AlarmSystem.h"
#include "UI.h"

// AppController.h
// Maps physical input (Buttons, RFID) to output

class AppController
{
  public:
    AppController(Buttons &btn, RFIDHandler &rfid, AlarmSystem &alm, UI &ui, DFRobotDFPlayerMini &player);

    // General input handler, internally calls individual input handlers
    void handleIn();

    // Individual input handlers
    void handleButtonIn();
    void handleRFIDIn();

  private:
    // In
    Buttons &_btn;
    RFIDHandler &_rfid;

    // Out
    AlarmSystem &_alm;
    UI &_ui;

    DFRobotDFPlayerMini &_player;

    // Timers
    unsigned long _lastPressedMillis;
};