#pragma once
#include "AlarmSystem.h"
#include "Buttons.h"
#include "Config.h"
#include "NetworkManager.h"
#include "Timekeeper.h"
#include <RTClib.h>
#include <TFT_eSPI.h>
#include <functional>

// UI.h
// Display updates and functions

enum class State
{
    Boot,
    Clock,
    Settings
}; // TODO: add implementation for State::Settings

class UI
{
  public:
    UI(TFT_eSPI &tft, Buttons &btn, Timekeeper &tk, NetworkManager &net);

    bool begin();

    void run();

    void setState(State state);
    State getState() const;

    // Boot state functions
    bool displayStartupStatus(bool rtcOK, bool rtcLostPower, bool playerOK, bool rfidOK);

    // Clock state functions
    void drawClockScreen(const DateTime &time, const WeatherData &weather);

    void updateTimeDisplay(const DateTime &time, bool isColon = true);
    void updateDateDisplay(const DateTime &time);
    void updateWeatherDisplay(const WeatherData &weather);
    void updateAlarmDisplay();

    void handleClockButtonIn();

    // Helpers
    void drawCenteredString(const char text[],
                            uint16_t textColor = Colors::TEXT_COLOR,
                            uint16_t bgColor = Colors::BACKGROUND_COLOR,
                            uint8_t font = 1,
                            uint8_t size = 2);
    void flashScreen(uint16_t flashColor, int flashDuration = 150);

    // For recieving alarm data from callback
    struct AlarmDisplayData
    {
        uint8_t hour;
        uint8_t minute;
        bool enabled;
        bool ringing;
    };

    // Callbacks
    using AlarmDataCallback = std::function<AlarmDisplayData()>;
    void setAlarmDataCallback(AlarmDataCallback cb);

  private:
    TFT_eSPI &_tft;
    State _state;

    Buttons &_btn;
    Timekeeper &_tk;
    NetworkManager &_net;

    AlarmDataCallback _alarmDataCb;
};