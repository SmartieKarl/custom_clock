#include "NetworkManager.h"
#include "Config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

NetworkManager::NetworkManager(RTC_DS3231 &rtc)
    : _rtc(rtc), _users(0), _persistent(false), _connecting(false), _mtx(NULL)
{
}

void NetworkManager::begin()
{
    _mtx = xSemaphoreCreateMutex();
    if (!_mtx)
        LOG.log("Warning: NetworkManager mutex initialization failed.");
}

bool NetworkManager::startWiFiSession()
{
    xSemaphoreTake(_mtx, portMAX_DELAY);

    // If already connecting, wait for other user to finish
    while (_connecting)
    {
        xSemaphoreGive(_mtx);
        vTaskDelay(pdMS_TO_TICKS(100));
        xSemaphoreTake(_mtx, portMAX_DELAY);
    }

    if (_users == 0 && WiFi.status() != WL_CONNECTED)
    {
        _connecting = true; // claim the connection attempt
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        unsigned long start = millis();
        xSemaphoreGive(_mtx);
        while (WiFi.status() != WL_CONNECTED && (millis() - start < 15000))
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        xSemaphoreTake(_mtx, portMAX_DELAY);

        _connecting = false; // release the claim

        if (WiFi.status() != WL_CONNECTED)
        {
            xSemaphoreGive(_mtx);
            return false;
        }
    }

    _users++;
    xSemaphoreGive(_mtx);
    return true;
}

void NetworkManager::endWiFiSession()
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    if (_users > 0)
        _users--;

    // Disconnect if no users and not in persistent mode
    if (_users == 0 && !_persistent)
    {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
    }

    xSemaphoreGive(_mtx);
}

void NetworkManager::setWiFiPersistent(bool persistent)
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    _persistent = persistent;
    Serial.print("WiFi persistent mode: ");
    Serial.println(persistent ? "ENABLED" : "DISABLED");
    xSemaphoreGive(_mtx);
}

bool NetworkManager::isWiFiPersistent() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    bool result = _persistent;
    xSemaphoreGive(_mtx);
    return result;
}

bool NetworkManager::fetchWeather()
{
    if (!startWiFiSession())
        return false;

    // Helper on fail state
    auto fail = [this]() -> bool
    {
        endWiFiSession();
        currentWeather.valid = false;
        return false;
    };

    // Build URL
    char url[256];
    snprintf(url, sizeof(url),
             "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=imperial",
             WEATHER_LOCATION, WEATHER_API_KEY);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode <= 0)
    {
        http.end();
        return fail();
    }

    // Read payload into buffer
    const size_t payloadSize = 1024;
    char payload[payloadSize];
    WiFiClient *stream = http.getStreamPtr();
    size_t len = stream->readBytes(payload, payloadSize - 1);
    payload[len] = '\0';

    http.end();

    JsonDocument doc; // Potential crash point: remove comment if OK
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
        return fail();

    JsonObject mainObj = doc["main"].as<JsonObject>();
    JsonArray weatherArr = doc["weather"].as<JsonArray>();
    if (!mainObj || !weatherArr || weatherArr.size() == 0)
        return fail();

    currentWeather.temperature = mainObj["temp"].as<float>();
    currentWeather.tempMin = mainObj["temp_min"].as<float>();
    currentWeather.tempMax = mainObj["temp_max"].as<float>();
    currentWeather.humidity = mainObj["humidity"].as<int>();

    strncpy(currentWeather.description, weatherArr[0]["description"], sizeof(currentWeather.description) - 1);
    currentWeather.description[sizeof(currentWeather.description) - 1] = '\0';

    strncpy(currentWeather.mainCondition, weatherArr[0]["main"], sizeof(currentWeather.mainCondition) - 1);
    currentWeather.mainCondition[sizeof(currentWeather.mainCondition) - 1] = '\0';

    endWiFiSession();
    currentWeather.valid = true;

    return true;
}

// synchronizes RTC time from NTP server
bool NetworkManager::syncRTCFromNTP()
{
    if (!startWiFiSession())
        return false;

    struct tm timeinfo = {};
    unsigned long start = millis();
    const unsigned long ntpTimeout = 10000;

    bool gotTime = false;
    while (millis() - start < ntpTimeout)
    {
        if (getLocalTime(&timeinfo))
        {
            gotTime = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (!gotTime)
    {
        endWiFiSession();
        return false;
    }

    DateTime dt(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);

    _rtc.adjust(dt);

    endWiFiSession();
    return true;
}