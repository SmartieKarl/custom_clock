#include "Timekeeper.h"

// Constructor
Timekeeper::Timekeeper(RTC_DS3231 &rtc)
    : _rtc(rtc), _curr(DateTime(2000, 1, 1, 0, 0, 0)),
      _prev(DateTime(2000, 1, 1, 0, 0, 0)), _mtx(NULL)
{
}

void Timekeeper::begin()
{
    _mtx = xSemaphoreCreateMutex();
    if (!_mtx)
        LOG.log("Warning! Timekeeper mutex initialization failed.");
    _curr = _rtc.now();
    _prev = _curr;
}
// Syncs software time with hardware time
void Timekeeper::update()
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    _prev = _curr;
    _curr = _rtc.now();
    xSemaphoreGive(_mtx);
}

// Returns cached time
DateTime Timekeeper::time() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    DateTime t = _curr;
    xSemaphoreGive(_mtx);
    return t;
}

// Returns whether time has changed
bool Timekeeper::tick() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool r = _curr != _prev;
    xSemaphoreGive(_mtx);
    return r;
}

// Returns whether second has changed
bool Timekeeper::secondTick() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool r = _curr.second() != _prev.second();
    xSemaphoreGive(_mtx);
    return r;
}

// Returns whether minute has changed
bool Timekeeper::minuteTick() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool r = _curr.minute() != _prev.minute();
    xSemaphoreGive(_mtx);
    return r;
}

// Returns whether hour has changed
bool Timekeeper::hourTick() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool r = _curr.hour() != _prev.hour();
    xSemaphoreGive(_mtx);
    return r;
}

// Returns whether day has changed
bool Timekeeper::dayTick() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool r = _curr.day() != _prev.day();
    xSemaphoreGive(_mtx);
    return r;
}