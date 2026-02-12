#include "player.h"
#include "rtc.h"

DFRobotDFPlayerMini player;
HardwareSerial mySoftwareSerial(1); // UART1 for ESP32 (DFPlayer)

// Initialize the DFPlayer Mini audio module
bool initializeDFPlayer(HardwareSerial &serial)
{
    serial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

    if (!player.begin(serial))
    {
        return false;
    }
    player.volume(PLAYER_VOLUME);
    return true;
}

// Stops audio playback
void stopAudioPlayback()
{
    player.stop();
}

// Plays the specified track number
void playTrack(int track, int folder, int vol)
{
    if (vol <= 30 && vol >= 0)
    {
        player.volume(vol);
    }
    if (folder == 0)
        player.playMp3Folder(track);
    else
        player.playFolder(folder, track);
}

// Loops specified track until told to stop.
void loopTrack(int track, int folder, int vol)
{
    if (vol <= 30 && vol >= 0)
    {
        player.volume(vol);
    }
    if (folder == 0)
    {
        player.playMp3Folder(track);
    }
    else
    {
        player.playFolder(folder, track);
    }
    player.enableLoop();
}

// Shuffles all mp3 files and plays them in random order.
void playAllOnShuffle()
{
    player.randomAll();
}

void setVolume(int vol)
{
    if (vol > 30 || vol < 0)
        return;
    player.volume(vol);
}

void loopPlayerFolder(int folder)
{
    player.loopFolder(folder);
}