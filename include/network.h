#pragma once
#include "domain.h"

//network module
//handles all network-related functions

extern WeatherData currentWeather;

void initWiFiManager();

bool startWiFiSession();
void endWiFiSession();

bool fetchWeather(WeatherData &weather);