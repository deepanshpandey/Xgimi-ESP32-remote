#include <Arduino.h>
#include "wifi_manager.h"
#include "ble_hid_remote.h"
#include "web_server.h"
#include "xgimi_keymap.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   XGIMI ESP32 BLE Remote Control Server  ");
    Serial.println("==========================================");

    // 1. Initialize BLE HID Remote Server
    Serial.printf("[BLE] Initializing BLE HID (%s, VID: 0x%04X, PID: 0x%04X)...\n", 
                  XGIMI_DEVICE_NAME, XGIMI_VENDOR_ID, XGIMI_PRODUCT_ID);
    BleRemote.begin(XGIMI_DEVICE_NAME, XGIMI_VENDOR_ID, XGIMI_PRODUCT_ID);

    // 2. Initialize Wi-Fi (Home Wi-Fi + SoftAP + mDNS)
    initWiFi();

    // 3. Initialize Embedded Web Application & API
    initWebServer();

    Serial.println("[SYSTEM] Ready!");
}

void loop() {
    ws.cleanupClients();
    processSerialWiFiCommands();
    delay(10);
}
