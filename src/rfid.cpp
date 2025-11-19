#include "rfid.h"
#include "display.h"

// Main RFID card checking function
void checkRFIDCard(MFRC522 &rfid, DFRobotDFPlayerMini &player, RTC_DS3231 &rtc, WeatherData &weather)
{
    // Check if a new card is present
    if (!rfid.PICC_IsNewCardPresent())
    {
        return; // No new card
    }

    // Try to read the card serial
    if (!rfid.PICC_ReadCardSerial())
    {
        return; // Failed to read
    }

    // Get card UID
    String cardUID = getCardUID(rfid);

    // Handle the card action directly
    if (cardUID == MASTER_CARD_UID)
    {
        handleAlarmCard(player, rtc, weather);
    }
    else
    {
        handleUnknownCard(cardUID, rtc, weather);
    }

    // Halt the card and stop encryption
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

// Handle alarm card actions
void handleAlarmCard(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc, WeatherData &weather)
{
    // Check if alarm is currently playing
    if (isAlarmTriggered())
    {
        // Visual feedback: RED flash for alarm stop
        flashScreen(TFT_RED, 200);
        stopAlarm(player, rtc);
    }
    else
    {
        // Alarm not playing - cycle through alarm times
        // Visual feedback: GREEN flash for settings change
        flashScreen(TFT_GREEN, 150);
        cycleAlarmTime();
    }

    // Restore normal display
    restoreDisplay(rtc, weather);
}

// Handle unknown cards
void handleUnknownCard(const String &cardUID, RTC_DS3231 &rtc, WeatherData &weather)
{
    // Visual feedback: YELLOW flash for unknown card
    flashScreen(TFT_YELLOW, 100);

    // Restore display
    restoreDisplay(rtc, weather);
}

// Get card UID as formatted string
String getCardUID(MFRC522 &rfid)
{
    String cardUID = "";
    for (byte i = 0; i < rfid.uid.size; i++)
    {
        if (rfid.uid.uidByte[i] < 0x10)
            cardUID += "0"; // Add leading zero
        cardUID += String(rfid.uid.uidByte[i], HEX);
    }
    cardUID.toUpperCase();
    return cardUID;
}

// Restore normal display after card actions
void restoreDisplay(RTC_DS3231 &rtc, const WeatherData &weather)
{
    delay(100); // Small delay to ensure flash screen completes
    DateTime now = rtc.now();
    updateTimeDisplay(now);
    updateWeatherDisplay(weather);
    updateAlarmDisplay();
}