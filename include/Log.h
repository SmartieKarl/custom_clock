#pragma once
#include <Arduino.h>
#include <Preferences.h>

#include "Timekeeper.h"

// Reference
static constexpr int LOG_SIZE = 32;
static constexpr int LOG_ENTRY_SIZE = 128;

class Timekeeper;
class Log
{
  public:
    Log(Timekeeper &tk);

    void begin();

    void log(const char *fmt, ...);
    bool pop(char *out);

    bool empty() const;
    int size() const;

    void clear();

    void printToSerial() const;
    void dumpRawBufferToSerial() const;

    void saveToFlash();
    void loadFromFlash();

  private:
    // Objects
    Timekeeper &_tk;

    char logQueue[LOG_SIZE][LOG_ENTRY_SIZE];
    int head = 0;
    int tail = 0;
    int count = 0;

    Preferences prefs;

    mutable SemaphoreHandle_t _mtx; // Mutex safety
};

// Universal Log object access
extern Log LOG;