#include "tasks.h"
#include "command_interface.h"
#include "display.h"
#include "log.h"
#include "network.h"
#include "rtc.h"
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

// Blynk callback functions here

// Command-response virtual pin
BLYNK_WRITE(V0)
{
    Serial.println("BLYNK_WRITE: started");
    const char *rsp = handleBlynkIn(param.asStr());
    if (rsp[0] != '\0') // Will be '/0' if the read command had the signature clock prefix: [CLK]:
    {
        // Write and notify
        Blynk.virtualWrite(V0, rsp);
        Blynk.logEvent("clock_reply");
        Serial.printf("rsp <%s> written to pin V0.", rsp);
    }
}

// On blynk connect, flush log to log pin (V1)
BLYNK_CONNECTED()
{
    Serial.println("BLYNK_CONNECTED: started.");
    Blynk.sendCmd(BLYNK_CMD_PING); // Outbound ping to prevent server-side idle timeout
    Blynk.syncVirtual(V0);         // Syncs last value from cmd stream for execution
    while (LOG.hasLog())
    {
        // Write to V1 (log stream)
        Blynk.virtualWrite(V1, LOG.pop());
    }
}

void timeSyncTask(void *)
{
    while (true)
    {
        syncRTCFromNTP();

        DateTime now = getNow();
        uint32_t secondsToMidnight = (23 - now.hour()) * 3600 +
                                     (59 - now.minute()) * 60 +
                                     (60 - now.second());

        vTaskDelay(pdMS_TO_TICKS(secondsToMidnight * 1000ULL));
    }
}

void weatherTask(void *)
{
    while (true)
    {
        if (fetchWeather(currentWeather))
            updateWeatherDisplay(currentWeather);

        DateTime now = getNow();
        uint32_t secondsToNext = 0;
        if (now.minute() < 30)
        {
            secondsToNext = (30 - now.minute()) * 60 - now.second();
        }
        else
        {
            secondsToNext = (60 - now.minute()) * 60 - now.second();
        }

        vTaskDelay(pdMS_TO_TICKS(secondsToNext * 1000ULL));
    }
}

void blynkTask(void *)
{
    Serial.println("BlynkTask: started");
    constexpr int MAX_RETRIES = 5;
    constexpr unsigned long BURST_MS = 20000; // 20s
    Blynk.config(BLYNK_AUTH);

    while (true)
    {
        bool persistent = isWiFiPersistent();

        // Connect to WiFi (retry connection if periodic)
        bool connected = false;
        int retries = persistent ? 1 : MAX_RETRIES;

        for (int i = 0; i < retries && !connected; i++)
        {
            Serial.println("BlynkTask: attempting to connect to WiFi");
            if (startWiFiSession())
            {
                Serial.println("BlynkTask: WiFi connected successfully.");
                connected = true;
            }
            else if (!persistent)
            {
                Serial.printf("BlynkTask: WiFi failed. Retrying... (%d/%d)\n", i + 1, MAX_RETRIES);
            }
        }

        // If connected, establish Blynk connection
        if (connected)
        {
            Blynk.connect(10000);
            if (Blynk.connected())
            {
                // Run Blynk
                if (persistent)
                {
                    // Persistent mode: run continuously until persistence ends
                    while (isWiFiPersistent())
                    {
                        Blynk.run();
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                    Serial.println("BlynkTask: Persistence ended, closing WiFi.");
                }
                else
                {
                    // Burst mode: run for 10 seconds then disconnect
                    unsigned long start = millis();
                    while (millis() - start < BURST_MS)
                    {
                        Blynk.run();
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                }
                Blynk.disconnect();
                endWiFiSession();
                Serial.println("BlynkTask: WiFi session ended.");
            }
        }

        // After disconnect / failed connect
        DateTime now = getNow();
        uint32_t secondsToNext = 0;
        if (now.minute() < 30)
        {
            secondsToNext = (30 - now.minute()) * 60 - now.second();
        }
        else
        {
            secondsToNext = (60 - now.minute()) * 60 - now.second();
        }

        unsigned long waitStart = millis();
        unsigned long waitMs = secondsToNext * 1000ULL;
        while (millis() - waitStart < waitMs)
        {
            if (isWiFiPersistent())
                break;                       // Exit early if persistence requested
            vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
        }
    }
}