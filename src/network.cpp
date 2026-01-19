#include "network.h"
#include "config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

WeatherData currentWeather = {0, 0, 0, 0, "", "", false};
static uint8_t wifiUsers = 0;
static SemaphoreHandle_t wifiMutex = NULL;


//initializes the WiFi mutex to avoid task conflicts
void initWiFiManager() {
    if (!wifiMutex) {
        wifiMutex = xSemaphoreCreateMutex();
    }
}

//connects to WiFi and manages multiple session requests
bool startWiFiSession() {
    if (!wifiMutex) initWiFiManager();

    xSemaphoreTake(wifiMutex, portMAX_DELAY);

    if (wifiUsers == 0)
    {
        //only connect if it's the first user
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

//ends a WiFi session, disconnecting if last user
void endWiFiSession() {
    xSemaphoreTake(wifiMutex, portMAX_DELAY);
    if (wifiUsers > 0) wifiUsers--;

    if (wifiUsers == 0) {
        // Only disconnect if it's the last user
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    xSemaphoreGive(wifiMutex);
}

//fetches weather data from OpenWeatherMap API
bool fetchWeather(WeatherData &weather)
{
    //connect to WiFi
    if (!startWiFiSession()) return false;

    //build API URL with proper URL encoding for city name
    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    String cityEncoded = String(WEATHER_CITY);
    cityEncoded.replace(" ", "%20"); //URL encode spaces
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
        return false;
    }

    String payload = http.getString();
    http.end();

    //parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        endWiFiSession();
        return false;
    }

    //validate Json data exists and is correct type
    if (!doc["main"].is<JsonObject>() ||
        !doc["main"]["temp"].is<float>() ||
        !doc["weather"].is<JsonArray>() ||
        doc["weather"].size() == 0 ||
        !doc["weather"][0]["description"].is<String>())
    {
        endWiFiSession();
        return false;
    }

    //extract weather data
    weather.temperature = doc["main"]["temp"];
    weather.tempMin = doc["main"]["temp_min"];
    weather.tempMax = doc["main"]["temp_max"];
    weather.humidity = doc["main"]["humidity"];
    weather.description = doc["weather"][0]["description"].as<String>();
    weather.mainCondition = doc["weather"][0]["main"].as<String>();

    endWiFiSession();

    return true;
}