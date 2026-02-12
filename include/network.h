#pragma once
#include "domain.h"

extern WeatherData currentWeather;

void initWiFiManager();

bool startWiFiSession();
void endWiFiSession();

bool fetchWeather(WeatherData &weather);

// WiFi persistence control
void setWiFiPersistent(bool persistent);
bool isWiFiPersistent();