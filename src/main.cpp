#include <Arduino.h>
#include <WiFi.h>
#include "ble_hid_remote.h"
#include "web_server.h"
#include "xgimi_keymap.h"

// Wi-Fi Configuration
const char* AP_SSID = "XGIMI-Remote-AP";
const char* AP_PASS = "xgimiremote";

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   XGIMI ESP32 BLE Remote Control Server  ");
    Serial.println("==========================================");

    // 1. Initialize Wi-Fi Access Point mode
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("[Wi-Fi] SoftAP Created! SSID: '%s'\n", AP_SSID);
    Serial.printf("[Wi-Fi] Web Remote URL: http://%s\n", apIP.toString().c_str());

    // 2. Initialize BLE HID Remote Server
    Serial.printf("[BLE] Initializing BLE HID (VID: 0x%04X, PID: 0x%04X)...\n", XGIMI_VENDOR_ID, XGIMI_PRODUCT_ID);
    BleRemote.begin(XGIMI_DEVICE_NAME, XGIMI_VENDOR_ID, XGIMI_PRODUCT_ID);

    // 3. Initialize Embedded Web Application & API
    initWebServer();

    Serial.println("[SYSTEM] Ready! Connect your phone to Wi-Fi 'XGIMI-Remote-AP'");
    Serial.println("[SYSTEM] Open http://192.168.4.1 in your phone browser to control XGIMI.");
}

void loop() {
    ws.cleanupClients();
    delay(10);
}
