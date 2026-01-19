/****************************
 * Alarm Clock Project
 * Written by Michael Marsh
 *
 * Hardware: ESP32 + ST7789 TFT + DS3231 RTC + DFPlayer Mini + RFID-RC522
 ****************************/

#include "brightness.h"
#include "button.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include "player.h"
#include "rfid.h"
#include "rtc.h"
#include "settings.h"
#include "settings_menu.h"

// FreeRTOS tasks
void weatherTask(void *);
void timeSyncTask(void *);

// Helpers
bool initializeHardware();
void handleClockButtons(ButtonState butIn);
void handleSettingsButtons(ButtonState butIn);

// App state
AppMode currentMode = AppMode::BOOT;

//========== ENTRY POINT ==========

void setup()
{
    // Load persistent user settings from flash
    uSet.load();

    initWiFiManager();
    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");

    if (!initializeHardware())
    {
        unsigned long lastAction = millis();
        const unsigned long interval = 1000;
        tftPrintText("Press button 1 to continue", TFT_YELLOW);
        while(!getButtonStates().btn1)
        {
            unsigned long now = millis();
            if (now - lastAction >= interval)
            {
                tftPrintText(".", TFT_YELLOW);
                lastAction = now;
            }
            delay(10);
        }
    }
    else
    {
        delay(1000);
    }

    currentMode = AppMode::CLOCK;
    drawClockScreen(getNow(), currentWeather);
    
    // Start background tasks
    xTaskCreate(weatherTask, "WeatherTask", 4096, NULL, 1, NULL);
    xTaskCreate(timeSyncTask, "TimeSyncTask", 4096, NULL, 1, NULL);
}

void loop()
{
    static uint8_t lastSecond = 255;
    static uint8_t lastMinute = 255;
    static uint8_t lastDay = 255;
    static ButtonState lastButtonState = {false, false, false, false};

    DateTime now = getNow();
    uint8_t currentSecond = now.second();
    uint8_t currentMinute = now.minute();
    uint8_t currentDay = now.day();

    // Get button state with debouncing
    ButtonState currentButtonState = getButtonStates();
    
    // Only process button press on rising edge (button just pressed)
    bool buttonJustPressed = currentButtonState.any() && !lastButtonState.any();
    
    if (buttonJustPressed)
    {
        switch (currentMode)
        {
            case AppMode::CLOCK:
                handleClockButtons(currentButtonState);
                break;
                
            case AppMode::SETTINGS:
                handleSettingsButtons(currentButtonState);
                break;
                
            default:
                break;
        }
    }
    
    lastButtonState = currentButtonState;

    // Clock mode operations
    if (currentMode == AppMode::CLOCK)
    {
        // Check for alarm card scan
        if (isAlarmRinging())
        {
            RFIDResult rfidResult = rfidCheckCard();
            if (rfidResult.event == RFIDEvent::ALARM_CARD)
            {
                setAlarmRinging(false);
                stopPlayback();
                flashScreen(TFT_GREEN, 500);
                drawClockScreen(now, currentWeather);
            }
        }

        // Daily updates
        if (currentDay != lastDay)
        {
            lastDay = currentDay;
            updateDateDisplay(now);
        }

        // Minutely updates
        if (currentMinute != lastMinute)
        {
            lastMinute = currentMinute;
            updateTimeDisplay(now);

            // Check alarm
            if(isAlarmTime())
            {
                updateAlarmDisplay();
                playAllOnShuffle();
            }
        }

        // Per-second updates (if needed)
        if (currentSecond != lastSecond)
        {
            lastSecond = currentSecond;
            // Add any per-second updates here
        }
    }

    // Update ambient brightness (works in all modes)
    updateAmbientBrightness();

    delay(LOOP_DELAY);
}

//========== BUTTON HANDLERS ==========

void handleClockButtons(ButtonState butIn)
{
    if (!butIn.singlePress()) return;

    if (butIn.btn1)
    {
        // Enter settings menu
        currentMode = AppMode::SETTINGS;
        settingsMenu.enter();
        Serial.println("Entering settings menu");
    }
    else if (butIn.btn2)
    {
        // Quick alarm toggle
        AlarmTime alm = getAlarm();
        setAlarm(alm.hour, alm.minute, !alm.enabled);
        updateAlarmDisplay();
        flashScreen(alm.enabled ? TFT_GREEN : TFT_RED, 150);
        Serial.println(alm.enabled ? "Alarm enabled" : "Alarm disabled");
    }
    else if (butIn.btn3)
    {
        // Manual weather update
        Serial.println("Requesting weather update");
        WeatherData weather;
        if (fetchWeather(weather))
        {
            currentWeather = weather;
            updateWeatherDisplay(currentWeather);
            flashScreen(TFT_GREEN, 150);
        }
        else
        {
            flashScreen(TFT_RED, 150);
        }
    }
    else if (butIn.btn4)
    {
        // Placeholder for future function
        Serial.println("Button 4 pressed");
        flashScreen(TFT_BLUE, 150);
    }
}

void handleSettingsButtons(ButtonState butIn)
{
    if (!butIn.singlePress()) return;

    if (butIn.btn1)  // Up/Increment
    {
        if (settingsMenu.isActive())
        {
            // If editing a value, increment it
            // Otherwise navigate up
            settingsMenu.navigateUp();
            settingsMenu.incrementValue();
        }
    }
    else if (butIn.btn2)  // Down/Decrement
    {
        if (settingsMenu.isActive())
        {
            settingsMenu.navigateDown();
            settingsMenu.decrementValue();
        }
    }
    else if (butIn.btn3)  // Back
    {
        settingsMenu.navigateBack();
        
        // Check if menu exited
        if (!settingsMenu.isActive())
        {
            currentMode = AppMode::CLOCK;
            drawClockScreen(getNow(), currentWeather);
            Serial.println("Exiting settings menu");
        }
    }
    else if (butIn.btn4)  // Select/Enter
    {
        settingsMenu.selectCurrent();
    }
}

//========== FREERTOS TASKS ==========

void weatherTask(void *parameter)
{
    while (true)
    {
        if (fetchWeather(currentWeather))
        {
            if (currentMode == AppMode::CLOCK)
            {
                updateWeatherDisplay(currentWeather);
            }
        }

        // Calculate delay until next half hour
        DateTime now = getNow();
        uint8_t minutes = now.minute();
        uint8_t seconds = now.second();
        int next = (minutes < 30) ? 30 : 60;
        uint16_t waitSeconds = (next - minutes) * 60 - seconds;
        
        vTaskDelay(pdMS_TO_TICKS(waitSeconds * 1000UL));
    }
}

void timeSyncTask(void *parameter)
{
    while (true)
    {
        syncRTCFromNTP();

        // Calculate delay until midnight
        DateTime now = getNow();
        uint32_t secondsToMidnight = (23 - now.hour()) * 3600 + 
                                     (59 - now.minute()) * 60 + 
                                     (60 - now.second());
        
        vTaskDelay(pdMS_TO_TICKS(secondsToMidnight * 1000UL));
    }
}

//========== HARDWARE INITIALIZATION ==========

bool initializeHardware()
{
    bool rtcOK = true;
    bool playerOK = true;
    bool rfidOK = true;

    // Initialize components
    initializeButtons();
    initializeDisplay();
    initializeBrightness();

    if (!initializeRTC())
    {
        rtcOK = false;
        Serial.println("ERROR: RTC initialization failed");
    }
    
    if (!initializeDFPlayer(mySoftwareSerial))
    {
        playerOK = false;
        Serial.println("ERROR: DFPlayer initialization failed");
    }
    
    if (!initializeRFID())
    {
        rfidOK = false;
        Serial.println("ERROR: RFID initialization failed");
    }

    return displayStartupStatus(rtcOK, playerOK, rfidOK);
}