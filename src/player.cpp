#include "player.h"
#include "rtc.h"

DFRobotDFPlayerMini player;
HardwareSerial mySoftwareSerial(1); // UART1 for ESP32 (DFPlayer)

//Initialize the DFPlayer Mini audio module
bool initializeDFPlayer(HardwareSerial &serial)
{
    serial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

    if (!player.begin(serial))
    {
        return false;
    }
    player.volume(27); //brownout safeguard (30 not recommended)
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

//Loops specified track until told to stop.
void loopTrack(int trackNumber)
{
    player.loop(trackNumber);
}

//Shuffles all mp3 files and plays them in random order.
void playAllOnShuffle()
{
    player.randomAll();
}