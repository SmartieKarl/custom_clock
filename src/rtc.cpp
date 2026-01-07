#include "rtc.h"
#include "config.h"
#include "network.h"
#include "display.h"
#include <Wire.h> // To read DS3231 control register

//Global RTC object
RTC_DS3231 rtc;

//Global alarm variables
static bool alarmRinging = false;
AlarmTime currentAlarm = {7, 0, false};


// Initialize the DS3231 RTC module
bool initializeRTC()
{
    if (!rtc.begin())
    {
        return false;
    }

    initializeRTCAlarm();
    if (rtc.lostPower())
    {
        rtc.adjust(DateTime(0, 1, 1, 0, 0, 0));
    }
    return true;
}

//synchronizes RTC time from NTP server
bool syncRTCFromNTP()
{
    if (!startWiFiSession())
        return false;

    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");

    struct tm timeinfo = {};
    unsigned long start = millis();
    const unsigned long ntpTimeout = 10000;

    bool gotTime = false;
    while (millis() - start < ntpTimeout)
    {
        if (getLocalTime(&timeinfo))
        {
            gotTime = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (!gotTime)
    {
        endWiFiSession();
        return false;
    }

    DateTime dt(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );

    rtc.adjust(dt);

    endWiFiSession();
    return true;
}

// Initialize alarm system with DS3231 hardware alarm
// Only alarm 1 is used in this implementation
void initializeRTCAlarm()
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
bool checkIfAlarmTime()
{
    if (!currentAlarm.enabled)
        return false;

    if (rtc.alarmFired(1) && !alarmRinging)
    {
        rtc.clearAlarm(1); //reset alarm flag
        
        // player.loop(uSet.alarmSong);
        setAlarmRinging(true);
        updateAlarmDisplay(); // Show alarm ringing status
        return true;
    }

    return false;
}

// Enable or disable alarm
void setAlarm(uint8_t hr, uint8_t min, bool enable)
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
bool isAlarmEnabled()
{
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_CONTROL);
    Wire.endTransmission(false);
    Wire.requestFrom(DS3231_ADDRESS, 1);
    uint8_t ctrl = Wire.available() ? Wire.read() : 0;
    return ctrl & 0x01; // Bit 0: Alarm 1 enabled
}

//Returns alarm ringing state
bool getAlarmRinging()
{
    return alarmRinging;
}

void setAlarmRinging(bool ringing)
{
    alarmRinging = ringing;
}

// Returns current alarm settings
AlarmTime getAlarm()
{
    return currentAlarm;
}

DateTime getNow()
{
    return rtc.now();
}