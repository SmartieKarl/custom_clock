#pragma once
#include <Arduino.h>
#include <MFRC522.h>

// RFIDHandler.h
// RFID hardware abstraction layer for simpler card reading and ID

class RFIDHandler
{
  public:
    RFIDHandler(MFRC522 &rfid);

    void poll();
    void getCardUID(char out[64]);

    //========== Callbacks ==========
    using RFIDCallback = std::function<void(const char*)>;
    // Registers a callback when the RFID scanner detects a card,
    // passes card UID as argument
    void onRFIDEvent(RFIDCallback cb) { _onRFIDEvent = cb; }

  private:
    MFRC522 &_rfid;

    RFIDCallback _onRFIDEvent;
};