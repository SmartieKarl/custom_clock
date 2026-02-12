#include "command_interface.h"
#include "display.h"
#include "network.h"
#include "player.h"
#include "rtc.h"
#include <cstring>

// ========== COMMAND HANDLERS ==========
// General
void cmdHelp(int argc, char *argv[]);
void cmdStatus(int argc, char *argv[]);

// Clock
void cmdAlarm(int argc, char *argv[]);

// Player
void cmdVol(int argc, char *argv[]);
void cmdPlay(int argc, char *argv[]);
void cmdStop(int argc, char *argv[]);

// Network
void cmdWiFiSession(int argc, char *argv[]);
void cmdSync(int argc, char *argv[]);

constexpr Command commands[] =
    {
        {"help", cmdHelp, "prints this index of commands and usage"},
        {"status", cmdStatus, "status"},
        {"alarm", cmdAlarm, "alarm <set> <hour><minute> || <toggle>"},
        {"vol", cmdVol, "vol <0-30>"},
        {"play", cmdPlay, "play <folder> <track> [vol = DEFAULT]"},
        {"stop", cmdStop, "stop"},
        {"sync", cmdSync, "sync <time> || <weather>"},
        {"wifisession", cmdWiFiSession, "wifisession <on> || <off>"}};

constexpr int MAX_ARGS = 8;
constexpr int CMD_IN_SIZE = 128; // max size of input buffer

// Command output buffer
constexpr size_t CMD_OUT_SIZE = 512;
static char cmdOut[CMD_OUT_SIZE];

// Macro to append onto cmdOut bufer
#define CMD_APPEND(fmt, ...)                \
    snprintf(cmdOut + strlen(cmdOut),       \
             CMD_OUT_SIZE - strlen(cmdOut), \
             fmt, ##__VA_ARGS__)

// Takes tokenized command line and dispatches command data to respective
// command function. Returns command output at const char*
void dispatchCommand(int argc, char *argv[])
{
    if (argc == 0)
        return;
    cmdOut[0] = '\0'; // Reset command buffer
    CMD_APPEND("[CLK]: ");

    for (const Command &cmd : commands)
    {
        if (strcmp(argv[0], cmd.name) == 0)
        {
            return cmd.handler(argc, argv);
        }
    }

    CMD_APPEND("command <%s> not recognized.", argv[0]);
    return;
}

// Processes input line and tokenizes it for the dispatcher. Returns
// command output from the cmd function as const char*
void processCommandLine(char *line)
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
// Converts AdafruitIO data into char* and passes it to the processor
const char *handleBlynkIn(const char *line)
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
    return cmdOut;
}

// Called every loop.
// Reads serial command line, converts into char* and passes to processor
void handleSerialIn()
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
                Serial.println(cmdOut);
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
void cmdHelp(int argc, char *argv[])
{
    CMD_APPEND("--------------------\nAVAILABLE COMMANDS:\n");
    for (const Command &cmd : commands)
    {
        CMD_APPEND("%s: %s\n", cmd.name, cmd.description);
    }
    CMD_APPEND("--------------------\n");
}

// Prints current time and date
void cmdStatus(int argc, char *argv[])
{
    DateTime now = getNow();
    CMD_APPEND("time: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
    CMD_APPEND("date: %02d/%02d/%04d\n", now.month(), now.day(), now.year());
}

// Either sets alarm at given time, or disables alarm
void cmdAlarm(int argc, char *argv[])
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

        setAlarm((int)hr, (int)min, true);
        CMD_APPEND("Alarm set to %02d:%02d", hr, min);
    }
    else if (strcmp(argv[1], "disable") == 0)
    {
        setAlarm(0, 0, false);
        CMD_APPEND("Alarm disabled");
    }
    else
    {
        CMD_APPEND("Usage: alarm <set hr min> || <disable>");
    }
}

// Sets audio volume to given int
void cmdVol(int argc, char *argv[])
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

    setVolume(v);
    CMD_APPEND("Volume set to %d", v);
}

// Plays track, and if folder and/or vol are provided, plays at folder at volume
void cmdPlay(int argc, char *argv[])
{
    if (argc < 2 || argc > 4)
    {
        CMD_APPEND("Usage: play <folder> <track> [volume = DEFAULT]");
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
        CMD_APPEND("Err: folder must be > 0");
        return;
    }
    folder = (int)f;

    // Track arg validation
    char *endptr_track;
    long t = strtol(argv[2], &endptr_track, 10);

    if (endptr_track == argv[2] || *endptr_track != '\0' || t <= 0)
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

    playTrack(track, folder, volume);
    CMD_APPEND("Now playing %d/%d @%d vol", folder, track, volume);
}

// Stops audio playback
void cmdStop(int argc, char *argv[])
{
    stopAudioPlayback();
    CMD_APPEND("Ok: Audio playback stopped.");
}

// Starts/stops persistent wifi session (useful for live remote commands)
void cmdWiFiSession(int argc, char *argv[])
{
    if (argc != 2)
    {
        CMD_APPEND("Usage: wifisession <on> || <off>");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        setWiFiPersistent(true);
        CMD_APPEND("Wifi session started.\nPolling frequency set to continuous.");
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        setWiFiPersistent(false);
        CMD_APPEND("Wifi session stopped.\nPolling frequency set to every :00 & :30.");
    }
    else
        CMD_APPEND("Err: arg was invalid (on || off)");
}

// Syncs time or weather data
void cmdSync(int argc, char *argv[])
{
    if (argc < 2)
    {
        CMD_APPEND("Usage: sync <time || weather>");
        return;
    }

    if (strcmp(argv[1], "time") == 0)
    {
        if (syncRTCFromNTP())
            CMD_APPEND("Time sync successful.");
        else
            CMD_APPEND("Err: unable to sync time to RTC.");
    }
    if (strcmp(argv[1], "weather") == 0)
    {
        if (fetchWeather(currentWeather))
        {
            updateWeatherDisplay(currentWeather);
            CMD_APPEND("Weather fetch successful.");
        }
        else
            CMD_APPEND("Err: unable to fetch weather data.");
    }
}