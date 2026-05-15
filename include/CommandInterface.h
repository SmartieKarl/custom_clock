#pragma once
#include "AlarmSystem.h"
#include "NetworkManager.h"
#include "Timekeeper.h"
#include "UI.h"
#include <DFRobotDFPlayerMini.h>

// command_interface.h
//  Abstraction class between received system commands and logic

class CommandInterface
{
  public:
    CommandInterface(DFRobotDFPlayerMini &player, Timekeeper &tk, UI &ui, NetworkManager &net, AlarmSystem &alm);

    // Source control
    const char *handleBlynkIn(const char *line);
    void handleSerialIn();

    // For manual command input
    void processCommandLine(char *line);

  private:
    void dispatchCommand(int argc, char *argv[]);

    // ========== COMMAND HANDLERS ==========
    // General
    void cmdHelp(int argc, char *argv[]);
    void cmdStatus(int argc, char *argv[]);
    void cmdTime(int argc, char *argv[]);
    void cmdLog(int argc, char *argv[]);

    // Alarm
    void cmdAlarm(int argc, char *argv[]);
    void cmdAlarmType(int argc, char *argv[]);

    // Player
    void cmdVol(int argc, char *argv[]);
    void cmdPlay(int argc, char *argv[]);
    void cmdStop(int argc, char *argv[]);

    // Network
    void cmdWiFiSession(int argc, char *argv[]);
    void cmdSync(int argc, char *argv[]);

  private:
    static constexpr size_t MAX_ARGS = 8;
    static constexpr size_t CMD_IN_SIZE = 128; // max size of input buffer
    static constexpr size_t CMD_OUT_SIZE = 512;

    static constexpr size_t NUM_COMMANDS = 11; // Update when new command is added!

    // Objects
    DFRobotDFPlayerMini &_player;
    Timekeeper &_tk;
    UI &_ui;
    NetworkManager &_net;
    AlarmSystem &_alm;

    using CommandHandler = void (CommandInterface::*)(int argc, char *argv[]);
    struct Command
    {
        const char *name;
        CommandHandler handler;
        const char *description;
    };
    // Command table
    const Command _commands[NUM_COMMANDS] = {
        {"help", &CommandInterface::cmdHelp, "prints this index of commands and usage"},
        {"status", &CommandInterface::cmdStatus, "status"},
        {"time", &CommandInterface::cmdTime, "time <set> <hour> <minute> <month> <day> <year>"},
        {"log", &CommandInterface::cmdLog, "log <log <message>> || <pop> || <size> || <printall> || <dumpbuffer> || <save> || <load>"},
        {"alarm", &CommandInterface::cmdAlarm, "alarm <set> <hour><minute> || <toggle>"},
        {"alarmtype", &CommandInterface::cmdAlarmType, "alarmtype <loud || normal || buzzer || all || int(trackNumber)> <vol>"},
        {"vol", &CommandInterface::cmdVol, "vol <0-30>"},
        {"play", &CommandInterface::cmdPlay, "play <folder> <track> [vol = DEFAULT]"},
        {"stop", &CommandInterface::cmdStop, "stop"},
        {"sync", &CommandInterface::cmdSync, "sync <time> || <weather>"},
        {"wifisession", &CommandInterface::cmdWiFiSession, "wifisession <on> || <off>"}};

    // Command output buffer
    char _cmdOut[CMD_OUT_SIZE];

    // Helpers
    bool parseLong(char *arg, long &out, const char *name);
};