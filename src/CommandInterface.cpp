#include "CommandInterface.h"
#include "Config.h"
#include <Arduino.h>
#include <cstring>

using namespace std;

// Handler type and command struct

#define CMD_APPEND(fmt, ...) \
    snprintf(_cmdOut + strlen(_cmdOut), CMD_OUT_SIZE - strlen(_cmdOut), fmt, ##__VA_ARGS__)

// Constructor
CommandInterface::CommandInterface(DFRobotDFPlayerMini &player, Timekeeper &tk, UI &ui, NetworkManager &net, AlarmSystem &alm)
    : _player(player), _tk(tk), _ui(ui), _net(net), _alm(alm) {}

// ========== CommandInterface member definitions ==========

// Takes tokenized command line and dispatches command data to respective command function.
void CommandInterface::dispatchCommand(int argc, char *argv[])
{
    if (argc == 0)
        return;
    _cmdOut[0] = '\0';     // Reset command buffer
    CMD_APPEND("[CLK]: "); // Append clock signaure to reply

    // Check if a valid command name was entered
    for (const Command &cmd : _commands)
        if (strcmp(argv[0], cmd.name) == 0)
            return (this->*cmd.handler)(argc, argv);

    CMD_APPEND("command <%s> not recognized.", argv[0]);
    return;
}

// Processes input line and tokenizes it for the dispatcher.
void CommandInterface::processCommandLine(char *line)
{
    for (char *p = line; *p; ++p)
        *p = tolower((unsigned char)*p);

    char *argv[MAX_ARGS];
    int argc = 0;

    // Tokenize on spaces
    char *token = strtok(line, " \t\r\n");
    while (token && argc < MAX_ARGS)
    {
        argv[argc++] = token;
        token = strtok(nullptr, " \t\r\n");
    }

    if (argc == 0)
        return;

    dispatchCommand(argc, argv);
}

// Called when network detects an incoming message from AdafruitIO.
const char *CommandInterface::handleBlynkIn(const char *line)
{
    if (!line)
        return "";

    char buf[CMD_IN_SIZE];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    // If prefix matches clock signature, ignore it
    if (strncmp(buf, "[CLK]:", 6) == 0)
    {
        return "";
    }
    processCommandLine(buf);
    return _cmdOut;
}

// Called every loop. Reads serial command line, converts into char* and passes to processor
void CommandInterface::handleSerialIn()
{
    static char inputBuffer[CMD_IN_SIZE];
    static uint8_t inputPos = 0;

    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (inputPos > 0)
            {
                inputBuffer[inputPos] = '\0';
                processCommandLine(inputBuffer);
                Serial.println(_cmdOut);
                inputPos = 0;
            }
        }
        else if (inputPos < sizeof(inputBuffer) - 1)
        {
            inputBuffer[inputPos++] = c;
        }
        else
        {
            inputPos = 0;
            Serial.println("ERR: input too long");
        }
    }
}

// Prints all available commands and usage
void CommandInterface::cmdHelp(int argc, char *argv[])
{
    CMD_APPEND("--------------------\nAVAILABLE COMMANDS:\n");
    for (const Command &cmd : _commands)
    {
        CMD_APPEND("%s: %s\n", cmd.name, cmd.description);
    }
    CMD_APPEND("--------------------\n");
}

// Prints current time and date
void CommandInterface::cmdStatus(int argc, char *argv[])
{
    DateTime now = _tk.time();
    CMD_APPEND("time: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
    CMD_APPEND("date: %02d/%02d/%04d\n", now.month(), now.day(), now.year());
}

// Either sets alarm at given time, or disables alarm
void CommandInterface::cmdAlarm(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_APPEND("Usage: alarm <set hr min> || <disable>");
        return;
    }

    if (strcmp(argv[1], "set") == 0)
    {
        if (argc != 4)
        {
            CMD_APPEND("Usage: alarm set <hr> <min>   (ex: alarm set 21 30)");
            return;
        }

        // Convert strings to ints with validation
        char *endptr_hr;
        long hr = strtol(argv[2], &endptr_hr, 10);

        char *endptr_min;
        long min = strtol(argv[3], &endptr_min, 10);

        // Check if conversion was successful and consumed the whole string
        if (endptr_hr == argv[2] || *endptr_hr != '\0' ||
            endptr_min == argv[3] || *endptr_min != '\0')
        {
            CMD_APPEND("Err: hour and minute must be valid numbers");
            return;
        }

        // Verify arg range
        if (hr < 0 || hr > 23)
        {
            CMD_APPEND("Err: hour must be between 0 and 23");
            return;
        }

        if (min < 0 || min > 59)
        {
            CMD_APPEND("Err: minute must be between 0 and 59");
            return;
        }

        _alm.setAlarm((int)hr, (int)min, true);
        CMD_APPEND("Alarm set to %02d:%02d", hr, min);
    }
    else if (strcmp(argv[1], "disable") == 0)
    {
        _alm.setAlarm(0, 0, false);
        CMD_APPEND("Alarm disabled");
    }
    else
    {
        CMD_APPEND("Usage: alarm <set hr min> || <disable>");
    }
}

// Configures the sound the clock plays when the alarm goes off
void CommandInterface::cmdAlarmType(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_APPEND("Usage: alarmtype <loud || normal || buzzer || all || int(trackNumber)> <vol>");
        return;
    }

    int alarmType = -1; // 0: loud, 1: normal, 2: buzzer, 3: all, 4: custom track
    int trackNum = 0;

    if (strcmp(argv[1], "loud") == 0)
        alarmType = 0;
    else if (strcmp(argv[1], "normal") == 0)
        alarmType = 1;
    else if (strcmp(argv[1], "buzzer") == 0)
        alarmType = 2;
    else if (strcmp(argv[1], "all") == 0)
        alarmType = 3;
    else
    {
        // Try to parse as int > 0
        char *endptr;
        long val = strtol(argv[1], &endptr, 10);
        if (endptr == argv[1] || *endptr != '\0' || val <= 0)
        {
            CMD_APPEND("Err: invalid alarm type.");
            return;
        }
        alarmType = 4;
        trackNum = (int)val;
    }

    int volume = PLAYER_VOLUME;
    if (argc >= 3)
    {
        char *endptr;
        long v = strtol(argv[2], &endptr, 10);
        if (endptr == argv[2] || *endptr != '\0' || v < 0 || v > 30)
        {
            CMD_APPEND("Err: volume must be between 0 and 30");
            return;
        }
        volume = (int)v;
    }

    CMD_APPEND("Alarm set to play ");
    // Perform action
    if (alarmType == 0)
    {
        _alm.trackRange = Tracks::loudSongs;
        CMD_APPEND("loud songs ");
    }
    if (alarmType == 1)
    {
        _alm.trackRange = Tracks::normalSongs;
        CMD_APPEND("normal songs ");
    }
    if (alarmType == 2)
    {
        _alm.trackRange = Tracks::buzzers;
        CMD_APPEND("buzzers ");
    }
    if (alarmType == 3)
    {
        _alm.trackRange = Tracks::allSongs;
        CMD_APPEND("all songs ");
    }
    if (alarmType == 4)
    {
        _alm.trackRange = {int(argv[1]), int(argv[1])};
        CMD_APPEND("track %s ", argv[1]);
    }

    _alm.normalVol = volume;
    CMD_APPEND("at volume %d", volume);
}

// Sets audio volume to given int
void CommandInterface::cmdVol(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_APPEND("Usage: vol <0-30>");
        return;
    }

    // Arg validation
    char *endptr_vol;
    long v = strtol(argv[1], &endptr_vol, 10);

    if (endptr_vol == argv[1] || *endptr_vol != '\0' || v < 0 || v > 30)
    {
        CMD_APPEND("Err: volume must be between 0 and 30");
        return;
    }
    v = (int)v;

    _player.volume(v);
    CMD_APPEND("Volume set to %d", v);
}

// Plays track, and if folder and/or vol are provided, plays at folder at volume
void CommandInterface::cmdPlay(int argc, char *argv[])
{
    if (argc < 2 || argc > 4)
    {
        CMD_APPEND("Usage: play <folder (0 is root)> <track> [volume = DEFAULT]");
        return;
    }

    // Default values for args
    int folder, track;
    int volume = PLAYER_VOLUME;

    // Folder arg validation
    char *endptr_folder;
    long f = strtol(argv[1], &endptr_folder, 10);

    if (endptr_folder == argv[1] || *endptr_folder != '\0' || f < 0)
    {
        CMD_APPEND("Err: folder cannot be negative");
        return;
    }
    folder = (int)f;

    // Track arg validation
    char *endptr_track;
    long t = strtol(argv[2], &endptr_track, 10);

    if (endptr_track == argv[2] || *endptr_track != '\0' || t < 0)
    {
        CMD_APPEND("Err: track must be > 0");
        return;
    }
    track = (int)t;

    // Volume arg validation
    if (argc == 4)
    {
        char *endptr_vol;
        long v = strtol(argv[3], &endptr_vol, 10);

        if (endptr_vol == argv[3] || *endptr_vol != '\0' || v < 0 || v > 30)
        {
            CMD_APPEND("Err: volume must be between 0 and 30");
            return;
        }
        volume = (int)v;
    }

    _player.volume(volume);

    if (folder == 0)
        _player.play(track);
    else
        _player.playFolder(folder, track);

    CMD_APPEND("Now playing %d/%d @%d vol", folder, track, volume);
}

// Stops audio playback
void CommandInterface::cmdStop(int argc, char *argv[])
{
    _player.stop();
    CMD_APPEND("Ok: Audio playback stopped.");
}

// Starts/stops persistent wifi session (useful for live remote commands)
void CommandInterface::cmdWiFiSession(int argc, char *argv[])
{
    if (argc != 2)
    {
        CMD_APPEND("Usage: wifisession <on> || <off>");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        _net.setWiFiPersistent(true);
        CMD_APPEND("Wifi session started.\nPolling frequency set to continuous.");
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        _net.setWiFiPersistent(false);
        CMD_APPEND("Wifi session stopped.\nPolling frequency set to every :00 & :30.");
    }
    else
        CMD_APPEND("Err: arg was invalid (on || off)");
}

// Syncs time or weather data
void CommandInterface::cmdSync(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_APPEND("Usage: sync <time || weather>");
        return;
    }

    if (strcmp(argv[1], "time") == 0)
    {
        if (_net.syncRTCFromNTP())
            CMD_APPEND("Time sync successful.");
        else
            CMD_APPEND("Err: unable to sync time to RTC.");
    }
    else if (strcmp(argv[1], "weather") == 0)
    {
        if (_net.fetchWeather())
        {
            _ui.updateWeatherDisplay(_net.currentWeather);
            CMD_APPEND("Weather fetch successful.");
        }
        else
            CMD_APPEND("Err: unable to fetch weather data.");
    }
    else
        CMD_APPEND("Err: arg was invalid (time || weather)");
}