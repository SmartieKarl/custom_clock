#include "NetworkManager.h"
#include "Config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_task_wdt.h> // To feed the dog on time-consuming functions

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
    // 30 sec timeout to prevent deadlock
    // This is the queue if another user is using the wifi
    if (xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000)) != pdTRUE)
        return false;

    unsigned long waitStart = millis();
    while (_connecting)
    {
        esp_task_wdt_reset();
        if (millis() - waitStart > 30000) // Timeout after 30s
        {
            xSemaphoreGive(_mtx);
            return false;
        }

        // Release mutex while waiting
        xSemaphoreGive(_mtx);
        vTaskDelay(pdMS_TO_TICKS(50));

        // Re-acquire
        if (xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000)) != pdTRUE)
            return false;
    }

    if (_users == 0 && WiFi.status() != WL_CONNECTED)
    {
        _connecting = true;
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        unsigned long start = millis();

        xSemaphoreGive(_mtx);
        while (WiFi.status() != WL_CONNECTED && (millis() - start < 15000))
        {
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        // Re-acquire mutex and update _connecting
        xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000));
        _connecting = false;

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
    xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000));
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
    xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000));
    _persistent = persistent;
    Serial.print("WiFi persistent mode: ");
    Serial.println(persistent ? "ENABLED" : "DISABLED");
    xSemaphoreGive(_mtx);
}

bool NetworkManager::isWiFiPersistent() const
{
    xSemaphoreTake(_mtx, pdMS_TO_TICKS(10000));
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
    http.setTimeout(5000);
    int httpCode = http.GET();

    esp_task_wdt_reset(); // Feed that dog

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
        esp_task_wdt_reset();
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