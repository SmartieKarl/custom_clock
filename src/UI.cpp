#include "UI.h"

// Constructor
UI::UI(TFT_eSPI &tft, Buttons &btn, Timekeeper &tk, NetworkManager &net) // TODO: consider removing network object access
    : _tft(tft), _state(State::Boot), _btn(btn), _tk(tk), _net(net)
{
}

// Init
bool UI::begin()
{
    _tft.setRotation(1); // landscape orientation
    _tft.fillScreen(BACKGROUND_COLOR);
    _tft.setCursor(0, 5);
    _tft.setTextColor(TEXT_COLOR);
    _tft.setTextSize(1);
    _tft.println("- Michael's totally wicked custom clock v0.52 -\n");

    return true;
}

// Updates UI state
void UI::run()
{
    switch (_state)
    {
    case State::Boot: // As of right now, this should not be possible.
    {
        DateTime time = _tk.time();
        _tft.fillScreen(BACKGROUND_COLOR);
        _tft.setTextSize(2);
        _tft.setCursor(8, 8);
        _tft.printf("Timestamp %02d:%02d:%02d %02d/%02d/%04d", time.hour(), time.minute(), time.second(), time.month(), time.day(), time.year());
        drawCenteredString("How did we get here?", TFT_YELLOW, BACKGROUND_COLOR, 2);
        break;
    }

    case State::Clock:
    {
        if (_tk.tick()) // If time has changed
        {
            DateTime time = _tk.time();
            if (_tk.secondTick()) // If second has changed
            {
                bool colon = (time.second() % 2 == 0); // true for even. false for odd
                updateTimeDisplay(time, colon);
            }
            /********** Unused **********
            if (_tk.minuteTick()) // If minute has changed
            {
            }
            if (_tk.hourTick()) // If hour has changed
            {
            }
            ****************************/
            if (_tk.dayTick()) // If day has changed
            {
                updateDateDisplay(time);
            }
        }
        handleClockButtonIn();
        break;
    }
    case State::Settings:
    {
        // TODO: add settings here pls
        break;
    }
    default:
        break;
    }
}

//==================== State Control ====================

// Sets and inits UI state
void UI::setState(State state)
{
    _state = state;

    switch (_state)
    {
    case State::Boot:
    {
        break;
    }
    case State::Clock:
    {
        drawClockScreen(_tk.time(), _net.currentWeather);
        break;
    }
    case State::Settings:
    {
        break;
    }
    default:
    {
        break;
    }
    }
}

// Returns UI state
State UI::getState() const
{
    return _state;
}

//==================== Boot State ====================

// Displays status of given hardware component bools, returns overall status bool
bool UI::displayStartupStatus(bool rtcOK, bool rtcLostPower, bool playerOK, bool rfidOK)
{
    _tft.setTextSize(1);
    _tft.setTextColor(TEXT_COLOR);

    _tft.println("----------------------------------------");
    _tft.println("Board status:\n");

    _tft.print("RTC ");
    _tft.println(rtcOK ? "OK" : "FAIL");

    if (rtcLostPower)
    {
        _tft.setTextColor(TFT_YELLOW);
        _tft.println("RTC experienced a power loss since last boot.");
        _tft.setTextColor(TEXT_COLOR);
    }

    _tft.print("DFPlayer ");
    _tft.println(playerOK ? "OK" : "FAIL");

    _tft.print("RFID ");
    _tft.println(rfidOK ? "OK" : "FAIL");

    if (rtcOK && playerOK && rfidOK)
    {
        _tft.setTextColor(TFT_GREEN);
        _tft.println("\nAll hardware responding.");
        _tft.setTextColor(TEXT_COLOR);
        return true;
    }
    else
    {
        _tft.setTextColor(TFT_RED);
        _tft.println("\nWarning: a hardware component is not responding.");
        _tft.println("Clock functions may not work as intended.");
        _tft.setTextColor(TEXT_COLOR);
        return false;
    }
}

//==================== Clock State ====================

// restores main screen display state
void UI::drawClockScreen(const DateTime &time, const WeatherData &weather)
{
    delay(100);
    _tft.fillScreen(BACKGROUND_COLOR);
    updateTimeDisplay(time);
    updateDateDisplay(time);
    updateWeatherDisplay(weather);
    updateAlarmDisplay();
}

// Updates time display
void UI::updateTimeDisplay(const DateTime &time, bool isColon)
{
    char buf[12];
    if (isColon)
        snprintf(buf, sizeof(buf), "%02d:%02d", time.hour(), time.minute());
    else
        snprintf(buf, sizeof(buf), "%02d %02d", time.hour(), time.minute());
    drawCenteredString(buf, TEXT_COLOR, BACKGROUND_COLOR, 7, 2);
}

// Updates date display
void UI::updateDateDisplay(const DateTime &time)
{
    _tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    _tft.setTextFont(1);
    _tft.setTextSize(1);
    _tft.setCursor(2, 16);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", time.month(), time.day(), time.year());
    _tft.print(buf);
}

// updates weather display
void UI::updateWeatherDisplay(const WeatherData &weather)
{
    _tft.fillRect(0, 208, 120, 32, BACKGROUND_COLOR);
    _tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    _tft.setTextFont(1);

    if (!weather.valid)
    {
        _tft.setTextSize(1);
        _tft.setCursor(2, 224);
        _tft.println("Weather Unavailable");
        return;
    }

    int16_t x = 2;
    int16_t y = 204;

    // current temp
    _tft.setTextSize(2);
    _tft.setCursor(x, y);

    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), "%d", (int)weather.temperature);
    _tft.print(tempBuf);

    // Measure printed temp width
    int16_t tempWidth = _tft.textWidth(tempBuf);

    // weather conditions and farenheit marker
    _tft.setTextSize(1);
    _tft.setCursor(x + tempWidth + 2, y + 6); // +6 aligns baselines nicely

    char buf[32];
    snprintf(buf, sizeof(buf), "F | H%d | L%d", (int)weather.tempMax, (int)weather.tempMin);
    _tft.print(buf);

    // hi/low temps
    _tft.setCursor(2, 224);
    _tft.println(weather.description);
}

// updates alarm display
void UI::updateAlarmDisplay()
{
    // Get alarm data via callback
    if (!_alarmDataCb)
        return;
    AlarmDisplayData alarm = _alarmDataCb();

    _tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    _tft.setTextFont(1);
    _tft.setTextSize(1);
    _tft.setTextDatum(BR_DATUM);

    if (alarm.enabled)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Alarm: %02d:%02d %s",
                 alarm.hour, alarm.minute,
                 alarm.ringing ? "RINGING" : "SET");
        _tft.drawString(buf, 314, 232);
    }
    else
    {
        _tft.drawString("   Alarm not set", 314, 232);
    }

    _tft.setTextDatum(TL_DATUM);
}

void UI::handleClockButtonIn()
{
    ButtonState s = _btn.getState();

    if (s.btn1.pressed)
    {
    }
    if (s.btn1.pressed)
    {
    }
    if (s.btn1.pressed)
    {
    }
    if (s.btn1.pressed)
    {
    }
}
//==================== Helpers ====================

// Draws given string centered on the display
void UI::drawCenteredString(const char text[],
                            uint16_t textColor,
                            uint16_t bgColor,
                            uint8_t font,
                            uint8_t size)
{
    _tft.setTextColor(textColor, bgColor);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextFont(font);
    _tft.setTextSize(size);
    _tft.drawString(text, _tft.width() / 2, _tft.height() / 2);
    _tft.setTextDatum(TL_DATUM); // Reset to default datum
}

// Flashes screen with color for set duration.
void UI::flashScreen(uint16_t flashColor, int flashDuration)
{

    _tft.fillScreen(flashColor);
    delay(flashDuration);
    drawClockScreen(_tk.time(), _net.currentWeather);
}

//==================== Callbacks ====================
void UI::setAlarmDataCallback(AlarmDataCallback cb)
{
    _alarmDataCb = cb;
}