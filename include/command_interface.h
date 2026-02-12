#pragma once

// command_interface.h
//  Abstraction layer between received system commands and logic

using CommandHandler = void (*)(int argc, char *argv[]);

struct Command
{
    const char *name;        // Command identifier
    CommandHandler handler;  // Name of respective handler function
    const char *description; // Help/usage for command
};

// Source control
const char *handleBlynkIn(const char *line);
void handleSerialIn();

void processCommandLine(char *line);