#include "alarm.h"
#include "display.h" // Include display and brightness control for alarm

// Global alarm state
bool alarmTriggered = false;
uint8_t currentAlarmIndex = 0;
static bool alarmEnabled = true;
static bool alarmManuallyStopped = false; // Track if alarm was manually stopped

// Predefined alarm times that can be cycled through
AlarmTime alarmTimes[] = {
    {5, 45, true},
    {6, 00, true},
    {6, 30, true},
    {7, 00, true},
    {7, 30, true},
    {8, 00, true}};

const uint8_t NUM_ALARM_TIMES = sizeof(alarmTimes) / sizeof(alarmTimes[0]);

// Initialize alarm system
void initializeAlarm()
{
    alarmTriggered = false;
    currentAlarmIndex = 0;
    alarmEnabled = true;
    alarmManuallyStopped = false;
}

// Check if it's alarm time and trigger if needed
bool checkAlarmTime(const DateTime &now, DFRobotDFPlayerMini &player)
{
    if (!alarmEnabled)
        return false;

    AlarmTime currentAlarm = alarmTimes[currentAlarmIndex];

    if (now.hour() == currentAlarm.hour &&
        now.minute() == currentAlarm.minute &&
        !alarmTriggered &&
        !alarmManuallyStopped && // Don't retrigger if manually stopped
        currentAlarm.enabled)
    {

        // Trigger alarm
        player.loop(ALARM_SONG); // This will loop the song automatically
        alarmTriggered = true;

        // Update display to show alarm is ringing
        setAlarmUpdateFlag(true);

        return true;
    }

    return false;
}

// Maintain alarm - ensure it keeps playing
void maintainAlarm(DFRobotDFPlayerMini &player)
{
    if (!alarmTriggered)
        return; // No alarm is currently triggered

    // The DFPlayer loop() function should handle continuous playback automatically
    // This function serves as a backup in case the loop stops working
    static unsigned long lastAlarmCheck = 0;
    unsigned long currentMillis = millis();

    // Check every 60 seconds and restart if needed (backup safety)
    if (currentMillis - lastAlarmCheck >= 60000)
    {                            // 1 minute
        player.loop(ALARM_SONG); // Restart loop to ensure it's still playing
        lastAlarmCheck = currentMillis;
    }
}

// Stop the alarm
void stopAlarm(DFRobotDFPlayerMini &player, RTC_DS3231 &rtc)
{
    if (alarmTriggered)
    {
        player.stop(); // Stop the looping alarm
        alarmTriggered = false;
        alarmManuallyStopped = true; // Mark as manually stopped

        // Update display to show alarm stopped
        setAlarmUpdateFlag(true);
    }
}

// Reset alarm flag when time passes - Fixed logic
void resetAlarmFlag(const DateTime &now, DFRobotDFPlayerMini &player)
{
    if (!alarmTriggered && !alarmManuallyStopped)
        return; // Nothing to reset

    AlarmTime currentAlarm = alarmTimes[currentAlarmIndex];

    // Check if we've moved past the alarm minute
    bool timePassed = (now.hour() != currentAlarm.hour) || (now.minute() != currentAlarm.minute);

    if (timePassed && alarmManuallyStopped)
    {
        // Time has passed and alarm was manually stopped - fully reset for next day
        alarmTriggered = false;
        alarmManuallyStopped = false;
        setAlarmUpdateFlag(true);
    }
    // If alarm is still triggered but time hasn't passed, keep it ringing
    // If alarm is triggered and time passed but wasn't manually stopped, keep ringing
}

// Cycle to the next alarm time
void cycleAlarmTime()
{
    currentAlarmIndex = (currentAlarmIndex + 1) % NUM_ALARM_TIMES;

    // Update display to show new alarm time
    setAlarmUpdateFlag(true);
}

// Get current alarm time
AlarmTime getCurrentAlarmTime()
{
    return alarmTimes[currentAlarmIndex];
}

// Check if alarm is currently triggered
bool isAlarmTriggered()
{
    return alarmTriggered;
}