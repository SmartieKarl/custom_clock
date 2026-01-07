#include "player.h"
#include "rtc.h"

DFRobotDFPlayerMini player;
HardwareSerial mySoftwareSerial(1); // UART1 for ESP32 (DFPlayer)

//Initialize the DFPlayer Mini audio module
bool initializeDFPlayer(DFRobotDFPlayerMini &player, HardwareSerial &serial)
{
    serial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

    if (!player.begin(serial))
    {
        return false;
    }
    return true;
}

//Stops audio playback
void stopPlayback()
{
        player.stop();
}

//Plays the specified track number
void playTrack(int trackNumber)
{
    player.play(trackNumber);
}