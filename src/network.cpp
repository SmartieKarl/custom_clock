#include "network.h"
#include "config.h"
#include <time.h>

bool connectWiFi(const char *ssid, const char *pass, unsigned long timeoutMs)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs)
    {
        delay(100);
    }

    return WiFi.status() == WL_CONNECTED; // false if wifi isn't connected
}

void disconnectWiFi()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

bool syncRTCFromNTP()
{
    static bool isFirstSync = true;

    if (WiFi.status() != WL_CONNECTED)
    {
        if (!connectWiFi(WIFI_SSID, WIFI_PASS, 15000))
        {
            return false;
        }
    }

    // Configure time with timezone and NTP servers
    configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");

    // Wait for time to be set
    struct tm timeinfo;
    unsigned long start = millis();
    const unsigned long ntpTimeout = 10000; // 10s
    bool gotTime = false;
    while (millis() - start < ntpTimeout)
    {
        if (getLocalTime(&timeinfo))
        {
            gotTime = true;
            break;
        }
        delay(200);
    }

    if (!gotTime)
    {
        return false;
    }

    // Set RTC to LOCAL time from the timezone-adjusted struct tm
    extern RTC_DS3231 rtc; // Reference to global RTC object
    DateTime dt(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);
    rtc.adjust(dt);

    // If this is not the first sync, disconnect WiFi to save power
    if (!isFirstSync)
    {
        disconnectWiFi();
    }
    else
    {

        isFirstSync = false; // Mark that first sync is complete
    }

    return true;
}

bool fetchWeather(WeatherData &weather)
{
    // Connect to WiFi if not already connected
    if (WiFi.status() != WL_CONNECTED)
    {
        if (!connectWiFi(WIFI_SSID, WIFI_PASS, 10000))
        {
            return false;
        }
    }

    // Build API URL with proper URL encoding for city name
    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    String cityEncoded = String(WEATHER_CITY);
    cityEncoded.replace(" ", "%20"); // URL encode spaces
    url += cityEncoded;
    url += ",";
    url += WEATHER_COUNTRY;
    url += "&appid=";
    url += WEATHER_API_KEY;
    url += "&units=imperial";

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode <= 0)
    {
        http.end();
        disconnectWiFi();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        disconnectWiFi();
        return false;
    }

    // Validate Json data exists and is correct type
    if (!doc["main"].is<JsonObject>() ||
        !doc["main"]["temp"].is<float>() ||
        !doc["weather"].is<JsonArray>() ||
        doc["weather"].size() == 0 ||
        !doc["weather"][0]["description"].is<String>())
    {
        disconnectWiFi();
        return false;
    }

    // Extract weather data
    weather.temperature = doc["main"]["temp"];
    weather.tempMin = doc["main"]["temp_min"];
    weather.tempMax = doc["main"]["temp_max"];
    weather.humidity = doc["main"]["humidity"];
    weather.description = doc["weather"][0]["description"].as<String>();
    weather.mainCondition = doc["weather"][0]["main"].as<String>();

    // Disconnect WiFi to save power
    disconnectWiFi();

    return true;
}