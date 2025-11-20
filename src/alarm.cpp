#include "alarm.h"
#include "display.h" // Include display and brightness control for alarm

// Global alarm state
bool alarmRinging = false;

// Initialize alarm system with DS3231 hardware alarm
void initializeAlarm(RTC_DS3231 &rtc)
{
    alarmRinging = false;

    rtc.clearAlarm(1);
    
    DateTime rtcAlarm = rtc.getAlarm1();
    //If RTC-side alarm does not match currentAlarm values, update it to match
    if (rtcAlarm.hour() != uSet.alarmTime.hour || rtcAlarm.minute() != uSet.alarmTime.minute)
    {
        rtc.setAlarm1(DateTime(0, 0, 0, uSet.alarmTime.hour, uSet.alarmTime.minute, 0), DS3231_A1_Hour);
    }
}

// Check if DS3231 hardware alarm fired and trigger if needed
bool checkAlarmTime(RTC_DS3231 &rtc, DFRobotDFPlayerMini &player)
{
    if (!uSet.alarmTime.enabled)
        return false;

    if (rtc.alarmFired(1) && !alarmRinging)
    {
        rtc.clearAlarm(1); //reset alarm flag
        
        player.loop(uSet.alarmSong);
        alarmRinging = true;
        updateAlarmDisplay(); // Show alarm ringing status
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

        updateAlarmDisplay(); // Show alarm stopped status
    }
}

// Set new alarm time and update DS3231 hardware alarm
void setAlarmTime(RTC_DS3231 &rtc, uint8_t hr, uint8_t min)
{
    uSet.alarmTime.hour = hr;
    uSet.alarmTime.minute = min;
    
    // Update DS3231 Alarm 1 with new time
    rtc.clearAlarm(1);
    rtc.setAlarm1(DateTime(0, 0, 0, hr, min, 0), DS3231_A1_Hour);
    
    updateAlarmDisplay(); // Show updated alarm time
}

// Get current alarm time
AlarmTime getAlarmTime()
{
    return uSet.alarmTime;
}

// Enable or disable alarm
void enableAlarm(RTC_DS3231 &rtc, bool enable)
{
    uSet.alarmTime.enabled = enable;
    
    if (enable)
    {
        // Re-enable alarm in DS3231
        rtc.clearAlarm(1);
        rtc.setAlarm1(DateTime(0, 0, 0, uSet.alarmTime.hour, uSet.alarmTime.minute, 0), DS3231_A1_Hour);
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