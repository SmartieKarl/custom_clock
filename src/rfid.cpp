#include <Arduino.h>
#include <SPI.h>

#include "rfid.h"
#include "config.h"
#include <MFRC522.h>

// Global RFID object
MFRC522 rfid(RFID_CS_PIN, RFID_RST_PIN);

// Initialize the MFRC522 RFID module.
bool initializeRFID()
{
    // Initialize RFID module
    SPI.begin();
    rfid.PCD_Init();

    // Check if RFID module is responding
    byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);

    if (version == 0x00 || version == 0xFF)
    {
        return false;
    }
    return true;
}

// Main RFID card checking function
RFIDResult rfidCheckCard()
{
    if (!rfid.PICC_IsNewCardPresent() ||
        !rfid.PICC_ReadCardSerial())
    {
        return {RFIDEvent::NONE, ""};
    }

    String uid = getCardUID();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    if (uid == ALARM_CARD_UID)
        return {RFIDEvent::ALARM_CARD, uid};

    return {RFIDEvent::UNKNOWN_CARD, uid};
}

// Get card UID as formatted string
String getCardUID()
{
    String cardUID = "";
    byte uidLen = rfid.uid.size;
    if (uidLen == 0 || uidLen > 10)
        return ""; // Defensive: avoid out-of-bounds
    for (byte i = 0; i < uidLen; i++)
    {
        if (rfid.uid.uidByte[i] < 0x10)
            cardUID += "0"; // Add leading zero
        cardUID += String(rfid.uid.uidByte[i], HEX);
    }
    cardUID.toUpperCase();
    return cardUID;
}