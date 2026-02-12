#include "network.h"
#include "config.h"
#include "display.h" // For wifi display status
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

WeatherData currentWeather = {0, 0, 0, 0, "", "", false};
static uint8_t wifiUsers = 0;
static SemaphoreHandle_t wifiMutex = NULL;
static bool wifiPersistent = false;
static String lastSeen; // Most recently logged

void initWiFiManager()
{
    if (!wifiMutex)
    {
        wifiMutex = xSemaphoreCreateMutex();
    }
}

bool startWiFiSession()
{
    bool wifiTurnedOn = false;

    if (!wifiMutex)
        initWiFiManager();

    xSemaphoreTake(wifiMutex, portMAX_DELAY);

    if (wifiUsers == 0 && WiFi.status() != WL_CONNECTED)
    {
        // Only connect if it's the first user
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        unsigned long endTime = millis() + 15000;
        while (WiFi.status() != WL_CONNECTED && millis() < endTime)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            xSemaphoreGive(wifiMutex);
            return false;
        }
    }

    wifiUsers++;
    xSemaphoreGive(wifiMutex);

    return true;
}

void endWiFiSession()
{
    bool wifiTurnedOff = false;

    xSemaphoreTake(wifiMutex, portMAX_DELAY);
    if (wifiUsers > 0)
        wifiUsers--;

    // Disconnect if no users and not in persistent mode
    if (wifiUsers == 0 && !wifiPersistent)
    {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
    }

    xSemaphoreGive(wifiMutex);
}

void setWiFiPersistent(bool persistent)
{
    xSemaphoreTake(wifiMutex, portMAX_DELAY);
    wifiPersistent = persistent;
    Serial.print("WiFi persistent mode: ");
    Serial.println(persistent ? "ENABLED" : "DISABLED");
    xSemaphoreGive(wifiMutex);
}

bool isWiFiPersistent()
{
    xSemaphoreTake(wifiMutex, portMAX_DELAY);
    bool result = wifiPersistent;
    xSemaphoreGive(wifiMutex);
    return result;
}

bool fetchWeather(WeatherData &weather)
{
    if (!startWiFiSession())
        return false;

    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    String cityEncoded = String(WEATHER_CITY);
    cityEncoded.replace(" ", "%20");
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
        endWiFiSession();
        weather.valid = false;
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        endWiFiSession();
        weather.valid = false;
        return false;
    }

    if (!doc["main"].is<JsonObject>() ||
        !doc["main"]["temp"].is<float>() ||
        !doc["weather"].is<JsonArray>() ||
        doc["weather"].size() == 0 ||
        !doc["weather"][0]["description"].is<String>())
    {
        endWiFiSession();
        weather.valid = false;
        return false;
    }

    weather.temperature = doc["main"]["temp"];
    weather.tempMin = doc["main"]["temp_min"];
    weather.tempMax = doc["main"]["temp_max"];
    weather.humidity = doc["main"]["humidity"];
    weather.description = doc["weather"][0]["description"].as<String>();
    weather.mainCondition = doc["weather"][0]["main"].as<String>();

    endWiFiSession();

    weather.valid = true;
    return true;
}