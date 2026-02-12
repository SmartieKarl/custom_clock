#include "log.h"
#include "rtc.h"
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

Log LOG;

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
    DateTime now = getNow();

    // Prepend timestamp
    snprintf(entry, sizeof(entry),
             "[%02d:%02d:%02d] %s",
             now.hour(), now.minute(), now.second(),
             msg);

    // Copy into circular buffer
    strncpy(logQueue[head], entry, LOG_ENTRY_SIZE - 1);
    logQueue[head][LOG_ENTRY_SIZE - 1] = '\0';

    head = (head + 1) % LOG_SIZE;
    if (count < LOG_SIZE)
        count++;
    else
        tail = (tail + 1) % LOG_SIZE;
}

const char *Log::pop()
{
    if (count == 0)
        return nullptr;

    const char *entry = logQueue[tail];
    tail = (tail + 1) % LOG_SIZE;
    count--;
    return entry;
}

bool Log::hasLog() const
{
    return count > 0;
}