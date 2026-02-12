#include "display.h"
#include "config.h"
#include "rtc.h"

// global TFT display object
TFT_eSPI tft = TFT_eSPI();

// initialize the ST7789 TFT display and show startup banner
bool initializeDisplay()
{
    tft.init();
    tft.setRotation(1); // landscape orientation
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setCursor(0, 5);
    tft.setTextColor(TEXT_COLOR);
    tft.setTextSize(1);
    tft.println("- Michael's totally wicked custom clock v0.52 -\n");

    return true;
}

// draws given string centered on the display
void drawCenteredString(const char text[],
                        uint16_t textColor,
                        uint16_t bgColor,
                        uint8_t font,
                        uint8_t size)
{
    tft.setTextColor(textColor, bgColor);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(font);
    tft.setTextSize(size);
    tft.drawString(text, tft.width() / 2, tft.height() / 2);
    tft.setTextDatum(TL_DATUM); // Reset to default datum
}

// draws label of given message for given button. Recommend font 1, size 2
void drawButtonLabel(uint8_t butNum,
                     const char label[],
                     uint16_t textColor,
                     uint16_t bgColor,
                     uint8_t font,
                     uint8_t size)
{
    uint8_t x = 0;
    uint8_t y = 0;

    switch (butNum)
    {
    case 1:
        tft.setTextDatum(TL_DATUM);
        x = 12;
        y = 16;
        break;
    case 2:
        tft.setTextDatum(TR_DATUM);
        x = tft.width() - 12;
        y = 16;
        break;
    case 3:
        tft.setTextDatum(BL_DATUM);
        x = 12;
        y = tft.height() - 16;
        break;
    case 4:
        tft.setTextDatum(BR_DATUM);
        x = tft.width() - 12;
        y = tft.height() - 16;
        break;
    default:
        return; // butNum was not valid
    }

    tft.setTextFont(font);
    tft.setTextSize(size);
    tft.setTextColor(textColor, bgColor);
    tft.drawString(label, x, y);
    tft.setTextDatum(TL_DATUM); // Reset to default datum
}

// flashes screen with color for set duration.
void flashScreen(uint16_t flashColor, int flashDuration = 150)
{

    tft.fillScreen(flashColor);
    delay(flashDuration);
    tft.fillScreen(BACKGROUND_COLOR);
}

// updates time display
void updateTimeDisplay(const DateTime &now, bool isColon)
{
    char buf[12];
    if (isColon)
        snprintf(buf, sizeof(buf), "%02d:%02d", now.hour(), now.minute());
    else
        snprintf(buf, sizeof(buf), "%02d %02d", now.hour(), now.minute());
    drawCenteredString(buf, TEXT_COLOR, BACKGROUND_COLOR, 7, 2);
}

// updates date display
void updateDateDisplay(const DateTime &now)
{
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setCursor(2, 16);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", now.month(), now.day(), now.year());
    tft.print(buf);
}

// updates weather display
void updateWeatherDisplay(const WeatherData &weather)
{
    tft.fillRect(0, 208, 120, 32, BACKGROUND_COLOR);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.setTextFont(1);

    if (!weather.valid)
    {
        tft.setTextSize(1);
        tft.setCursor(2, 224);
        tft.println("Weather Unavailable");
        return;
    }

    int16_t x = 2;
    int16_t y = 204;

    // current temp
    tft.setTextSize(2);
    tft.setCursor(x, y);

    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), "%d", (int)weather.temperature);
    tft.print(tempBuf);

    // Measure printed temp width
    int16_t tempWidth = tft.textWidth(tempBuf);

    // weather conditions and farenheit marker
    tft.setTextSize(1);
    tft.setCursor(x + tempWidth + 2, y + 6); // +6 aligns baselines nicely

    char buf[32];
    snprintf(buf, sizeof(buf), "F | H%d | L%d", (int)weather.tempMax, (int)weather.tempMin);
    tft.print(buf);

    // hi/low temps
    tft.setCursor(2, 224);
    tft.println(weather.description);
}

// updates alarm display
void updateAlarmDisplay()
{
    AlarmTime alarm = getAlarm();
    bool isRinging = isAlarmRinging();

    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextDatum(BR_DATUM);

    if (alarm.enabled)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Alarm: %02d:%02d %s",
                 alarm.hour, alarm.minute,
                 isRinging ? "RINGING" : "SET");
        tft.drawString(buf, 314, 232);
    }
    else
    {
        tft.drawString("   Alarm not set", 314, 232);
    }

    tft.setTextDatum(TL_DATUM);
}

// restores main screen display state
void drawClockScreen(const DateTime &now, const WeatherData &weather)
{
    delay(100);
    tft.fillScreen(BACKGROUND_COLOR);
    updateTimeDisplay(now);
    updateDateDisplay(now);
    updateWeatherDisplay(weather);
    updateAlarmDisplay();
}

// draws settings menu display with given labels
void drawSettingsMenu(const char labCent[],
                      const char lab1[],
                      const char lab2[],
                      const char lab3[],
                      const char lab4[],
                      uint16_t textColor,
                      uint16_t bgColor)
{
    tft.fillScreen(bgColor);
    drawCenteredString(labCent, textColor, bgColor, 4, 2);
    drawButtonLabel(1, lab1, textColor, bgColor);
    drawButtonLabel(2, lab2, textColor, bgColor);
    drawButtonLabel(3, lab3, textColor, bgColor);
    drawButtonLabel(4, lab4, textColor, bgColor);
}

// Displays status of given hardware component bools, returns overall status bool
bool displayStartupStatus(bool rtcOK, bool playerOK, bool rfidOK)
{
    tft.println("----------------------------------------");
    tft.println("Board status:\n");

    tft.setTextSize(1);
    tft.print("RTC ");
    tft.println(rtcOK ? "OK" : "FAIL");

    tft.print("DFPlayer ");
    tft.println(playerOK ? "OK" : "FAIL");

    tft.print("RFID ");
    tft.println(rfidOK ? "OK" : "FAIL");

    if (rtcOK && playerOK && rfidOK)
    {
        tft.setTextColor(TFT_GREEN);
        tft.println("\nAll hardware responding.");
        tft.setTextColor(TEXT_COLOR);
        return true;
    }
    else
    {
        tft.setTextColor(TFT_RED);
        tft.println("\nWarning: a hardware component is not responding.");
        tft.println("Clock functions may not work as intended.");
        tft.setTextColor(TEXT_COLOR);
        return false;
    }
}

// Helper to print a basic command line to the tft display with given message and color.
void tftPrintLine(const char *message, uint16_t color)
{
    tft.setTextColor(color);
    tft.println(message);
    tft.setTextColor(TEXT_COLOR);
}

// Helper to print basic text to the tft display with given message and color.
void tftPrintText(const char *message, uint16_t color)
{
    tft.setTextColor(color);
    tft.print(message);
    tft.setTextColor(TEXT_COLOR);
}