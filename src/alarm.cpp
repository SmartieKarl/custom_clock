#include "alarm.h"
#include "display.h" // Include display and brightness control for alarm

// Global alarm state
bool alarmRinging = false;
AlarmTime currentAlarm = {7, 0, true}; // Default: 7:00 AM, enabled

// Initialize alarm system with DS3231 hardware alarm
void initializeAlarm(RTC_DS3231 &rtc)
{
    alarmRinging = false;

    rtc.clearAlarm(1);
    
    // Configure DS3231 Alarm 1 for daily alarm at current alarm time
    // DS3231_A1_Hour matches when hours and minutes match (daily alarm)
    rtc.setAlarm1(DateTime(0, 0, 0, currentAlarm.hour, currentAlarm.minute, 0), DS3231_A1_Hour);
}

// Check if DS3231 hardware alarm fired and trigger if needed
bool checkAlarmTime(RTC_DS3231 &rtc, DFRobotDFPlayerMini &player)
{
    if (!currentAlarm.enabled)
        return false;

    if (rtc.alarmFired(1) && !alarmRinging)
    {
        rtc.clearAlarm(1);
        
        player.loop(userSettings.alarmSong);
        alarmRinging = true;
        updateAlarmDisplay();
        return true;
    }

    return false;
}

// Stop the alarm
void stopAlarm(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc)
{
    if (alarmRinging)
    {
        player.stop();
        alarmRinging = false;

        // Update display to show alarm stopped
        updateAlarmDisplay();
    }
}

// Set new alarm time and update DS3231 hardware alarm
void setAlarmTime(RTC_DS3231 &rtc, uint8_t hour, uint8_t minute)
{
    currentAlarm.hour = hour;
    currentAlarm.minute = minute;
    
    // Update DS3231 Alarm 1 with new time
    rtc.clearAlarm(1);
    rtc.setAlarm1(DateTime(0, 0, 0, hour, minute, 0), DS3231_A1_Hour);
    
    // Update display to show new alarm time
    updateAlarmDisplay();
}

// Get current alarm time
AlarmTime getAlarmTime()
{
    return currentAlarm;
}

// Enable or disable alarm
void enableAlarm(RTC_DS3231 &rtc, bool enable)
{
    currentAlarm.enabled = enable;
    
    if (enable)
    {
        // Re-enable alarm in DS3231
        rtc.clearAlarm(1);
        rtc.setAlarm1(DateTime(0, 0, 0, currentAlarm.hour, currentAlarm.minute, 0), DS3231_A1_Hour);
    }
    else
    {
        // Disable alarm in DS3231
        rtc.clearAlarm(1);
        rtc.disableAlarm(1);
    }
    
    updateAlarmDisplay();
}

bool isAlarmRinging()
{
    return alarmRinging;
}