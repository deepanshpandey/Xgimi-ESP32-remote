#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void initWiFi();
void processSerialWiFiCommands();
bool connectHomeWiFi(const String& ssid, const String& password);
String getHomeWiFiSSID();
String getHomeWiFiIP();
bool isHomeWiFiConnected();

#endif // WIFI_MANAGER_H
