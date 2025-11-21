#include "alarm.h"
#include "display.h" // Include display and brightness control for alarm
#include <Wire.h> // To read DS3231 control register

// Global alarm state
AlarmTime currentAlarm = {0, 0, false}; //default alarm settings, alarm @midnight, disabled
bool alarmRinging = false;

// Initialize alarm system with DS3231 hardware alarm
// Only alarm 1 is used in this implementation
void initializeAlarm(RTC_DS3231 &rtc)
{
    alarmRinging = false;

    rtc.clearAlarm(1);
    
    // if RTC lost power, reset alarm to defaults
    if (rtc.lostPower())
    {
        rtc.setAlarm1(DateTime(0, 0, 0, currentAlarm.hour, currentAlarm.minute, 0), DS3231_A1_Hour);
        if (!currentAlarm.enabled)
        {
            rtc.disableAlarm(1);
        }
    }
    // Load current alarm settings from DS3231
    DateTime rtcAlarm = rtc.getAlarm1();
    currentAlarm.hour = rtcAlarm.hour();
    currentAlarm.minute = rtcAlarm.minute();
    currentAlarm.enabled = isAlarmEnabled();
}

// Check if DS3231 hardware alarm fired and trigger if needed
bool checkAlarmTime(RTC_DS3231 &rtc, DFRobotDFPlayerMini &player)
{
    if (!currentAlarm.enabled)
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

// Get current alarm info
AlarmTime getAlarm()
{
    return currentAlarm;
}

// Enable or disable alarm
void setAlarm(RTC_DS3231 &rtc, uint8_t hr, uint8_t min, bool enable)
{
    currentAlarm.hour = hr;
    currentAlarm.minute = min;
    currentAlarm.enabled = enable;
    
    if (enable)
    {
        // Re-enable alarm in DS3231
        rtc.clearAlarm(1);
        rtc.setAlarm1(DateTime(0, 0, 0, hr, min, 0), DS3231_A1_Hour);
    }
    else
    {
        // Disable alarm in DS3231
        rtc.clearAlarm(1);
        rtc.disableAlarm(1);
    }
    
    updateAlarmDisplay();
}


// Read DS3231 control register to check alarm enable status.
//returns alarm 1 enable state
bool isAlarmEnabled() {
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_CONTROL);
    Wire.endTransmission(false);
    Wire.requestFrom(DS3231_ADDRESS, 1);
    uint8_t ctrl = Wire.available() ? Wire.read() : 0;
    return ctrl & 0x01; // Bit 0: Alarm 1 enabled
}

bool isAlarmRinging()
{
    return alarmRinging;
}