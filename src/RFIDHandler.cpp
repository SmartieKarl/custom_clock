#include "RFIDHandler.h"
#include "Config.h"

// Constructor
RFIDHandler::RFIDHandler(MFRC522 &rfid)
    : _rfid(rfid)
{
}

// Main RFID card checking function, returns void
// and instead passes UID through callbacks
void RFIDHandler::poll()
{
    if (!_rfid.PICC_IsNewCardPresent() ||
        !_rfid.PICC_ReadCardSerial())
    {
        return;
    }

    char uid[64];
    getCardUID(uid);

    _rfid.PICC_HaltA();
    _rfid.PCD_StopCrypto1();

    if (_onRFIDEvent)
        _onRFIDEvent(uid);
}

// Get card UID as formatted buffer
void RFIDHandler::getCardUID(char out[64])
{
    byte uidLen = _rfid.uid.size;
    if(uidLen == 0 || uidLen > 10) {
        out[0] = '\0';
        return;
    }

    char* ptr = out;
    size_t remaining = 64;

    for(byte i = 0; i < uidLen; i++) {
        int written = snprintf(ptr, remaining, "%02X", _rfid.uid.uidByte[i]);
        if(written < 0 || written >= remaining)
            break;
        ptr += written;
        remaining -= written;
    }

    *ptr = '\0';
}