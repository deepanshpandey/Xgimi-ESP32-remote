#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// =============================================================================
// Home Wi-Fi Configuration File
// 
// Enter your Home Wi-Fi SSID and Password below to connect the ESP32 to your
// local network. If left empty (""), the ESP32 will run in SoftAP mode only.
// =============================================================================

#define HOME_WIFI_SSID     ""       // <-- Enter your Wi-Fi name, e.g. "HomeNetwork"
#define HOME_WIFI_PASSWORD ""       // <-- Enter your Wi-Fi password, e.g. "SecretPassword123"

// mDNS address: access your remote at http://xgimi-remote.local on home Wi-Fi!
#define MDNS_HOSTNAME      "xgimi-remote"

// Access Point fallback / concurrent AP settings
#define AP_SSID            "XGIMI-Remote-AP"
#define AP_PASSWORD        "xgimiremote"

#endif // WIFI_CONFIG_H
