// #include "rtc.h"
// #include "config.h"
// #include "display.h"
// #include "network.h"
// #include <Wire.h> // To read DS3231 control register

// // synchronizes RTC time from NTP server
// bool syncRTCFromNTP()
// {
//     if (!startWiFiSession())
//         return false;

//     struct tm timeinfo = {};
//     unsigned long start = millis();
//     const unsigned long ntpTimeout = 10000;

//     bool gotTime = false;
//     while (millis() - start < ntpTimeout)
//     {
//         if (getLocalTime(&timeinfo))
//         {
//             gotTime = true;
//             break;
//         }
//         vTaskDelay(pdMS_TO_TICKS(200));
//     }

//     if (!gotTime)
//     {
//         endWiFiSession();
//         return false;
//     }

//     DateTime dt(
//         timeinfo.tm_year + 1900,
//         timeinfo.tm_mon + 1,
//         timeinfo.tm_mday,
//         timeinfo.tm_hour,
//         timeinfo.tm_min,
//         timeinfo.tm_sec);

//     rtc.adjust(dt);

//     endWiFiSession();
//     return true;
// }

// // Initialize alarm system with DS3231 hardware alarm
// // Only alarm 1 is used in this implementation
// void initializeRTCAlarm()
// {
//     DateTime rtcAlarm = rtc.getAlarm1();
//     currentAlarm.hour = rtcAlarm.hour();
//     currentAlarm.minute = rtcAlarm.minute();
//     currentAlarm.enabled = isAlarmEnabled();

//     if (currentAlarm.enabled && rtc.alarmFired(1))
//         alarmRinging = true;

//     if (rtc.lostPower())
//     {
//         rtc.setAlarm1(DateTime(0, 0, 0, currentAlarm.hour, currentAlarm.minute, 0), DS3231_A1_Hour);

//         if (!currentAlarm.enabled)
//             rtc.disableAlarm(1);
//     }
// }

// // Enable or disable alarm
// void setAlarm(uint8_t hr, uint8_t min, bool enable)
// {
//     currentAlarm.hour = hr;
//     currentAlarm.minute = min;
//     currentAlarm.enabled = enable;

//     if (enable)
//     {
//         // Re-enable alarm in DS3231
//         rtc.clearAlarm(1);
//         rtc.setAlarm1(DateTime(0, 0, 0, hr, min, 0), DS3231_A1_Hour);
//     }
//     else
//     {
//         // Disable alarm in DS3231
//         rtc.clearAlarm(1);
//         rtc.disableAlarm(1);
//     }

//     updateAlarmDisplay();
// }

// DateTime getNow()
// {
//     return rtc.now();
// }