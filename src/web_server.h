#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void initWebServer();
void handleWsMessage(void* arg, uint8_t* data, size_t len);

extern AsyncWebServer server;
extern AsyncWebSocket ws;

#endif // WEB_SERVER_H
