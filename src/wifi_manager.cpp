#include "wifi_manager.h"
#include "wifi_config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>

static Preferences prefs;
static String activeHomeSSID = "";
static String activeHomePass = "";
static String inputBuffer = "";

bool connectHomeWiFi(const String& ssid, const String& password) {
    if (ssid.length() == 0) {
        return false;
    }

    activeHomeSSID = ssid;
    activeHomePass = password;

    // Save to persistent flash memory (NVS)
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();

    Serial.printf("[Wi-Fi] Connecting to Home Wi-Fi: '%s'...\n", ssid.c_str());
    WiFi.disconnect(false, false);
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        IPAddress staIP = WiFi.localIP();
        Serial.println("==================================================");
        Serial.printf("[Wi-Fi] Connected to Home Wi-Fi!\n");
        Serial.printf("[Wi-Fi] IP Address: http://%s\n", staIP.toString().c_str());
        
        if (MDNS.begin(MDNS_HOSTNAME)) {
            Serial.printf("[Wi-Fi] mDNS URL:   http://%s.local\n", MDNS_HOSTNAME);
        }
        Serial.println("==================================================");
        return true;
    } else {
        Serial.printf("[Wi-Fi] Could not connect to '%s' (Timeout). Continuing with SoftAP mode.\n", ssid.c_str());
        return false;
    }
}

void initWiFi() {
    // 1. Dual Mode: Access Point + Station
    WiFi.mode(WIFI_AP_STA);

    // 2. Start Access Point
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress apIP = WiFi.softAPIP();
    Serial.println("--------------------------------------------------");
    Serial.printf("[Wi-Fi] SoftAP SSID:      '%s'\n", AP_SSID);
    Serial.printf("[Wi-Fi] SoftAP Password:  '%s'\n", AP_PASSWORD);
    Serial.printf("[Wi-Fi] Direct AP URL:    http://%s\n", apIP.toString().c_str());
    Serial.println("--------------------------------------------------");

    // 3. Check for saved credentials in Preferences (NVS)
    prefs.begin("wifi", true);
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");
    prefs.end();

    // Fallback to compile-time wifi_config.h if not set in NVS
    if (savedSSID.length() == 0 && strlen(HOME_WIFI_SSID) > 0) {
        savedSSID = HOME_WIFI_SSID;
        savedPass = HOME_WIFI_PASSWORD;
    }

    if (savedSSID.length() > 0) {
        connectHomeWiFi(savedSSID, savedPass);
    } else {
        Serial.println("[Wi-Fi] No Home Wi-Fi credentials set yet.");
        Serial.println("[Wi-Fi] Configure via include/wifi_config.h, WebApp, or USB CLI: 'SET_WIFI:<ssid>,<password>'");
    }
}

void processSerialWiFiCommands() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                inputBuffer.trim();
                if (inputBuffer.startsWith("SET_WIFI:")) {
                    String payload = inputBuffer.substring(9);
                    int commaIdx = payload.indexOf(',');
                    if (commaIdx > 0) {
                        String newSSID = payload.substring(0, commaIdx);
                        String newPass = payload.substring(commaIdx + 1);
                        Serial.printf("[USB-CLI] Received Wi-Fi credentials for '%s'\n", newSSID.c_str());
                        connectHomeWiFi(newSSID, newPass);
                    } else {
                        Serial.println("[USB-CLI] Error: Format must be SET_WIFI:ssid,password");
                    }
                } else if (inputBuffer.equalsIgnoreCase("GET_WIFI") || inputBuffer.equalsIgnoreCase("STATUS")) {
                    Serial.printf("[Wi-Fi Status] Connected: %s | SSID: '%s' | IP: %s | AP IP: %s\n",
                        WiFi.status() == WL_CONNECTED ? "YES" : "NO",
                        WiFi.SSID().c_str(),
                        WiFi.localIP().toString().c_str(),
                        WiFi.softAPIP().toString().c_str());
                }
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
}

String getHomeWiFiSSID() {
    return WiFi.status() == WL_CONNECTED ? WiFi.SSID() : activeHomeSSID;
}

String getHomeWiFiIP() {
    return WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Not Connected";
}

bool isHomeWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}
