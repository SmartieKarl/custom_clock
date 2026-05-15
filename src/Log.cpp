#include "Log.h"
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

Log::Log(Timekeeper &tk) : _tk(tk) {}

void Log::begin()
{
    _mtx = xSemaphoreCreateMutex();
    if (!_mtx)
        Serial.println("Warning: Log mutex initialization failed.");
}

// Appends given formatted text to log buffer
void Log::log(const char *fmt, ...)
{
    char msg[LOG_ENTRY_SIZE];   // formatted message
    char entry[LOG_ENTRY_SIZE]; // timestamp + message

    // Format user message
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // Get timestamp
    DateTime time = _tk.time();

    // Prepend timestamp
    snprintf(entry, sizeof(entry),
             "[%02d/%02d/%04d %02d:%02d:%02d]: %s",
             time.month(), time.day(), time.year(),
             time.hour(), time.minute(), time.second(),
             msg);

    xSemaphoreTake(_mtx, portMAX_DELAY);
    // Copy into circular buffer
    strncpy(logQueue[head], entry, LOG_ENTRY_SIZE - 1);
    logQueue[head][LOG_ENTRY_SIZE - 1] = '\0';

    head = (head + 1) % LOG_SIZE;
    if (count < LOG_SIZE)
        count++;
    else
        tail = (tail + 1) % LOG_SIZE;
    xSemaphoreGive(_mtx);
}

// Pops off the most recent log, writes it to arg out, and returns whether pop was a succes or not.
bool Log::pop(char *out)
{
    xSemaphoreTake(_mtx, portMAX_DELAY);

    if (count == 0)
    {
        xSemaphoreGive(_mtx);
        return false;
    }

    strncpy(out, logQueue[tail], LOG_ENTRY_SIZE - 1);
    out[LOG_ENTRY_SIZE - 1] = '\0';

    tail = (tail + 1) % LOG_SIZE;
    count--;

    xSemaphoreGive(_mtx);

    return true;
}

// returns whether log is empty or not
bool Log::empty() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool result = (count == 0);
    xSemaphoreGive(_mtx);

    return result;
}

int Log::size() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    int result = count;
    xSemaphoreGive(_mtx);

    return result;
}

void Log::clear()
{
    xSemaphoreTake(_mtx, portMAX_DELAY);

    head = 0;
    tail = 0;
    count = 0;

    // Optional but recommended: wipe memory so dumpRawBuffer shows clean state
    for (size_t i = 0; i < LOG_SIZE; i++)
    {
        logQueue[i][0] = '\0';
    }

    xSemaphoreGive(_mtx);
}

// Debug stuff

// Prints all active entries in log to Serial. Non-destructive. Not thread safe!
void Log::printToSerial() const
{
    for (size_t i = 0; i < count; i++)
    {
        size_t index = (tail + i) % LOG_SIZE;
        Serial.println(logQueue[index]);
    }
}

// Dumps the entire log buffer to serial. Useful if you need to find data already taken out of the log.
void Log::dumpRawBufferToSerial() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);

    Serial.println("=== RAW LOG BUFFER ===");

    for (size_t i = 0; i < LOG_SIZE; i++)
    {
        Serial.print("[");
        Serial.print(i);
        Serial.print("] ");

        // Show empty slots clearly
        if (logQueue[i][0] == '\0')
            Serial.println("<empty>");
        else
            Serial.println(logQueue[i]);
    }

    Serial.println("======================");

    Serial.printf(
        "head=%u tail=%u count=%u\n",
        head, tail, count);

    xSemaphoreGive(_mtx);
}

// Saves current log snapshot to preferences
void Log::saveToFlash()
{
    xSemaphoreTake(_mtx, portMAX_DELAY);

    prefs.begin("log", false);

    prefs.putUChar("count", count);
    prefs.putUChar("head", head);
    prefs.putUChar("tail", tail);

    for (size_t i = 0; i < LOG_SIZE; i++)
    {
        char key[8];
        snprintf(key, sizeof(key), "l%02u", i);
        prefs.putString(key, logQueue[i]);
    }

    prefs.end();

    xSemaphoreGive(_mtx);
}

// Loads saved log snapshot from preferences. Overwrites runtime log!
void Log::loadFromFlash()
{
    prefs.begin("log", true);

    count = prefs.getUChar("count", 0);
    head = prefs.getUChar("head", 0);
    tail = prefs.getUChar("tail", 0);

    for (size_t i = 0; i < LOG_SIZE; i++)
    {
        char key[8];
        snprintf(key, sizeof(key), "l%02u", i);

        String s = prefs.getString(key, "");
        strncpy(logQueue[i], s.c_str(), LOG_ENTRY_SIZE - 1);
        logQueue[i][LOG_ENTRY_SIZE - 1] = '\0';
    }

    prefs.end();
}