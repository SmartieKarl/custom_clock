#include "AlarmSystem.h"
#include "Config.h"
#include <Wire.h>

// Constructor
AlarmSystem::AlarmSystem(RTC_DS3231 &rtc, Timekeeper &tk, DFRobotDFPlayerMini &player)
    : _rtc(rtc), _tk(tk), _player(player), _ringing(false), _alarm({0, 00, false})
{
}

// Init alarm system using DS3231 hardware alarm
// Only alarm 1 is used in this implementation (TOFO: Add alarm 2 support)
void AlarmSystem::begin()
{
    DateTime rtcAlarm = _rtc.getAlarm1();
    _alarm.hour = rtcAlarm.hour();
    _alarm.minute = rtcAlarm.minute();
    _alarm.enabled = _isRTCAlarmEnabled();
}

void AlarmSystem::run()
{
    if (isAlarmTime())
    {
        _player.volume(normalVol);
        // Plays random song in track range (default is normal songs)
        _player.play(random(trackRange.first, trackRange.second + 1));
        //_player.loop(Tracks::buzzerTrack);

        _alarmMillis = millis(); // Set for snooze alarm

        // Event callback
        if (_onAlarmEvent)
            _onAlarmEvent();
    }

    // Snooze alarm: plays another song every 5 minutes until dismissed
    if (_ringing && _fiveMinutesPassed())
    {
        _player.volume(25);
        _player.play(random(Tracks::loudSongs.first, Tracks::loudSongs.second + 1)); // Loud folder
    }
}

AlarmTime AlarmSystem::getAlarm()
{
    return _alarm;
}

void AlarmSystem::setAlarm(uint8_t hr, uint8_t min, bool enable)
{
    _alarm = {hr, min, enable};

    if (enable)
    {
        // Re-enable alarm in DS3231
        _rtc.clearAlarm(1);
        _rtc.setAlarm1(DateTime(0, 0, 0, hr, min, 0), DS3231_A1_Hour);
    }
    else
    {
        // Disable alarm in DS3231
        _rtc.clearAlarm(1);
        _rtc.disableAlarm(1);
    }

    // Event callback
    if (_onAlarmEvent)
        _onAlarmEvent();
}

void AlarmSystem::dismissAlarm()
{
    _player.stop();
    _player.volume(PLAYER_VOLUME);

    _rtc.clearAlarm(1);
    _ringing = false;

    // Event callback
    if (_onAlarmEvent)
        _onAlarmEvent();
}

bool AlarmSystem::isRinging()
{
    return _ringing;
}

// Check if DS3231 hardware alarm fired and trigger if needed
bool AlarmSystem::isAlarmTime()
{
    if (!_alarm.enabled)
        return false;

    if (_rtc.alarmFired(1) && !_ringing)
    {
        _ringing = true;
        return true;
    }

    return false;
}

// Read DS3231 alarm register to determine whether hardware alarm is enabled
bool AlarmSystem::_isRTCAlarmEnabled()
{
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_CONTROL);
    Wire.endTransmission(false);
    Wire.requestFrom(DS3231_ADDRESS, 1);
    uint8_t ctrl = Wire.available() ? Wire.read() : 0;
    return ctrl & 0x01; // Bit 0: Alarm 1 enabled
}

// Using _alarmMillis, checks if 5 minutes has passed since last synced with millis().
bool AlarmSystem::_fiveMinutesPassed()
{
    const unsigned long FIVE_MINUTES = 5UL * 60UL * 1000UL;
    if (millis() - _alarmMillis >= FIVE_MINUTES)
    {
        _alarmMillis = millis(); // reset for next interval
        return true;
    }
    return false;
}