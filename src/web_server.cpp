#include "web_server.h"
#include "web_assets.h"
#include "xgimi_keymap.h"
#include "ble_hid_remote.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static bool triggerKeyByAction(const String& actionName) {
    for (size_t i = 0; i < XGIMI_KEYMAP_SIZE; i++) {
        if (actionName.equalsIgnoreCase(XGIMI_KEYMAP[i].actionName)) {
            BleRemote.sendKey(XGIMI_KEYMAP[i].hidPage, XGIMI_KEYMAP[i].hidCode);
            return true;
        }
    }
    Serial.printf("[WEB] Warning: Unknown action '%s'\n", actionName.c_str());
    return false;
}

static void sendPairingStatusJson(AsyncWebSocketClient* client = nullptr) {
    StaticJsonDocument<256> doc;
    doc["type"] = "pairing_status";
    doc["connected"] = BleRemote.isConnected();
    doc["advertising"] = BleRemote.isAdvertising();
    doc["bonded_count"] = BleRemote.getBondedCount();
    doc["peer_address"] = BleRemote.getConnectedAddress();
    
    String response;
    serializeJson(doc, response);
    if (client) {
        client->text(response);
    } else {
        ws.textAll(response);
    }
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        sendPairingStatusJson(client);
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WS] Client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (!error) {
                if (doc.containsKey("action")) {
                    String action = doc["action"].as<String>();
                    triggerKeyByAction(action);
                } else if (doc.containsKey("type") && doc["type"] == "get_status") {
                    sendPairingStatusJson(client);
                }
            }
        }
    }
}

void initWebServer() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // Serve web app homepage directly from PROGMEM (zero-copy flash streaming)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", (const uint8_t*)INDEX_HTML, INDEX_HTML_LEN);
    });

    // PWA Manifest, Service Worker & App Icon
    server.on("/manifest.json", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "application/manifest+json", (const uint8_t*)MANIFEST_JSON, MANIFEST_JSON_LEN);
    });

    server.on("/sw.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "application/javascript", (const uint8_t*)SW_JS, SW_JS_LEN);
    });

    server.on("/icon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "image/svg+xml", (const uint8_t*)ICON_SVG, ICON_SVG_LEN);
    });

    // REST API Endpoint: POST or GET /api/press?action=FOCUS_AUTO
    server.on("/api/press", HTTP_ANY, [](AsyncWebServerRequest *request){
        String action = "";
        if (request->hasParam("action")) {
            action = request->getParam("action")->value();
        }

        StaticJsonDocument<128> doc;
        if (action.length() > 0 && triggerKeyByAction(action)) {
            doc["status"] = "ok";
            doc["action"] = action;
        } else {
            doc["status"] = "error";
            doc["message"] = "Invalid or missing action parameter";
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // REST API Pairing Endpoints
    server.on("/api/pair/start", HTTP_POST, [](AsyncWebServerRequest *request){
        BleRemote.startPairingMode(60);
        sendPairingStatusJson();

        StaticJsonDocument<128> doc;
        doc["status"] = "ok";
        doc["message"] = "BLE Pairing Advertising started for 60s";
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/pair/clear", HTTP_POST, [](AsyncWebServerRequest *request){
        BleRemote.clearBonds();
        sendPairingStatusJson();

        StaticJsonDocument<128> doc;
        doc["status"] = "ok";
        doc["message"] = "Cleared all bonded Bluetooth devices";
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/pair/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["connected"] = BleRemote.isConnected();
        doc["advertising"] = BleRemote.isAdvertising();
        doc["bonded_count"] = BleRemote.getBondedCount();
        doc["peer_address"] = BleRemote.getConnectedAddress();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/pair/pin", HTTP_POST, [](AsyncWebServerRequest *request){
        String pin = "";
        if (request->hasParam("pin", true)) {
            pin = request->getParam("pin", true)->value();
        }

        Serial.printf("[PAIRING] Wi-Fi PIN Code received: '%s'\n", pin.c_str());
        StaticJsonDocument<128> doc;
        doc["status"] = "ok";
        doc["pin"] = pin;
        doc["message"] = "PIN received and processed";
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // API Endpoint to get keymap definitions
    server.on("/api/keymap", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(4096);
        JsonArray arr = doc.to<JsonArray>();
        for (size_t i = 0; i < XGIMI_KEYMAP_SIZE; i++) {
            JsonObject obj = arr.createNestedObject();
            obj["action"] = XGIMI_KEYMAP[i].actionName;
            obj["linux_code"] = XGIMI_KEYMAP[i].linuxCode;
            obj["hid_page"] = XGIMI_KEYMAP[i].hidPage;
            obj["hid_code"] = XGIMI_KEYMAP[i].hidCode;
            obj["description"] = XGIMI_KEYMAP[i].description;
        }
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Wi-Fi Configuration Endpoints
    server.on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["connected"] = isHomeWiFiConnected();
        doc["ssid"] = getHomeWiFiSSID();
        doc["ip"] = getHomeWiFiIP();
        doc["mdns"] = "http://xgimi-remote.local";
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/wifi/save", HTTP_POST, [](AsyncWebServerRequest *request){
        String ssid = "";
        String pass = "";
        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
        }
        if (request->hasParam("pass", true)) {
            pass = request->getParam("pass", true)->value();
        }

        bool success = connectHomeWiFi(ssid, pass);
        StaticJsonDocument<256> doc;
        doc["status"] = success ? "ok" : "error";
        doc["connected"] = success;
        doc["ip"] = getHomeWiFiIP();
        doc["message"] = success ? "Connected to Home Wi-Fi!" : "Could not connect to specified Wi-Fi";
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.begin();
    Serial.println("[WEB] HTTP & WebSocket Server started on port 80");
}
