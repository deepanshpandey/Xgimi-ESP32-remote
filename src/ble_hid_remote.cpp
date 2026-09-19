#include "ble_hid_remote.h"
#include "xgimi_keymap.h"

extern "C" int ble_svc_gap_device_appearance_set(uint16_t appearance);

BLEHidRemoteServer BleRemote;

// HID Report Descriptor matching standard Keyboard + Consumer Control
static const uint8_t HID_REPORT_DESCRIPTOR[] = {
  // Keyboard (Report ID 1)
  0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,       // USAGE (Keyboard)
  0xA1, 0x01,       // COLLECTION (Application)
  0x85, 0x01,       //   REPORT_ID (1)
  0x05, 0x07,       //   USAGE_PAGE (Keyboard/Keypad)
  0x19, 0xE0,       //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xE7,       //   USAGE_MAXIMUM (Keyboard Right GUI)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x25, 0x01,       //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,       //   REPORT_SIZE (1)
  0x95, 0x08,       //   REPORT_COUNT (8)
  0x81, 0x02,       //   INPUT (Data,Var,Abs) ; Modifier byte
  0x95, 0x01,       //   REPORT_COUNT (1)
  0x75, 0x08,       //   REPORT_SIZE (8)
  0x81, 0x01,       //   INPUT (Cnst,Ary,Abs) ; Reserved byte
  0x95, 0x06,       //   REPORT_COUNT (6)
  0x75, 0x08,       //   REPORT_SIZE (8)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x25, 0x80,       //   LOGICAL_MAXIMUM (128)
  0x05, 0x07,       //   USAGE_PAGE (Keyboard/Keypad)
  0x19, 0x00,       //   USAGE_MINIMUM (0)
  0x29, 0x80,       //   USAGE_MAXIMUM (128)
  0x81, 0x00,       //   INPUT (Data,Ary,Abs) ; Key array (6 bytes)
  0xC0,             // END_COLLECTION

  // Consumer Control (Report ID 2)
  0x05, 0x0C,       // USAGE_PAGE (Consumer Devices)
  0x09, 0x01,       // USAGE (Consumer Control)
  0xA1, 0x01,       // COLLECTION (Application)
  0x85, 0x02,       //   REPORT_ID (2)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x26, 0xFF, 0x03, //   LOGICAL_MAXIMUM (1023)
  0x19, 0x00,       //   USAGE_MINIMUM (0)
  0x2A, 0xFF, 0x03, //   USAGE_MAXIMUM (1023)
  0x75, 0x10,       //   REPORT_SIZE (16)
  0x95, 0x01,       //   REPORT_COUNT (1)
  0x81, 0x00,       //   INPUT (Data,Ary,Abs)
  0xC0              // END_COLLECTION
};

BLEHidRemoteServer::BLEHidRemoteServer() 
    : connected(false), advertising(false), currentConnId(0), hid(nullptr), inputKeyboard(nullptr), inputConsumer(nullptr) {}

void BLEHidRemoteServer::begin(const char* deviceName, uint16_t vid, uint16_t pid) {
    NimBLEDevice::init(deviceName);
    
    // Explicitly set GAP Appearance characteristic (0x2A01) to Generic Remote Control (0x0180 = 384)
    ble_svc_gap_device_appearance_set(0x0180);

    // Enable BLE Security (Bonding + MITM + Secure Connections)
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    NimBLEDevice::setSecurityCallbacks(this);

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(this);

    hid = new NimBLEHIDDevice(pServer);
    inputKeyboard = hid->inputReport(1); // Report ID 1
    inputConsumer = hid->inputReport(2); // Report ID 2

    hid->manufacturer("XGIMI");
    hid->pnp(0x02, vid, pid, 0x0100);    // Vendor ID source: Bluetooth SIG
    hid->hidInfo(0x00, 0x01);             // Country code 0, Normal connectable
    hid->reportMap((uint8_t*)HID_REPORT_DESCRIPTOR, sizeof(HID_REPORT_DESCRIPTOR));
    hid->setBatteryLevel(100);            // Populate Battery Service (0x180F / 0x2A19)
    hid->startServices();

    startPairingMode(60);
    Serial.printf("[BLE] NimBLE HID Server initialized (%s, VID: 0x%04X, PID: 0x%04X)\n", deviceName, vid, pid);
}

void BLEHidRemoteServer::startPairingMode(uint32_t durationSeconds) {
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(0x0180); // 0x0180 Remote Control
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->addServiceUUID(hid->batteryService()->getUUID());
    pAdvertising->start(durationSeconds);
    advertising = true;
    Serial.printf("[BLE] Pairing Mode Enabled! Advertising for %u seconds...\n", durationSeconds);
}

void BLEHidRemoteServer::stopPairingMode() {
    NimBLEDevice::getAdvertising()->stop();
    advertising = false;
    Serial.println("[BLE] Stopped Advertising.");
}

void BLEHidRemoteServer::clearBonds() {
    int numBonds = NimBLEDevice::getNumBonds();
    NimBLEDevice::deleteAllBonds();
    Serial.printf("[BLE] Cleared %d stored Bluetooth bonds from memory.\n", numBonds);
}

int BLEHidRemoteServer::getBondedCount() const {
    return NimBLEDevice::getNumBonds();
}

String BLEHidRemoteServer::getConnectedAddress() const {
    if (!connected) return "None";
    NimBLEServer* pServer = NimBLEDevice::getServer();
    if (pServer) {
        return pServer->getPeerInfo(currentConnId).getAddress().toString().c_str();
    }
    return "Connected";
}

void BLEHidRemoteServer::onConnect(NimBLEServer* pServer) {
    connected = true;
    advertising = false;
    currentConnId = pServer->getPeerInfo(0).getConnHandle();
    
    // Request fast 7.5ms - 15ms BLE connection interval for instant, low-latency button response
    pServer->updateConnParams(currentConnId, 6, 12, 0, 200);
    
    Serial.println("[BLE] Connection Established with XGIMI Device (Low-Latency Mode Active)!");
}

void BLEHidRemoteServer::onDisconnect(NimBLEServer* pServer) {
    connected = false;
    Serial.println("[BLE] Device Disconnected.");
}

// NimBLE Security Callbacks
uint32_t BLEHidRemoteServer::onPassKeyRequest() {
    Serial.println("[BLE Security] Passkey requested (Just Works / No I/O)");
    return 123456;
}

void BLEHidRemoteServer::onPassKeyNotify(uint32_t pass_key) {
    Serial.printf("[BLE Security] Passkey Notification: %06u\n", pass_key);
}

bool BLEHidRemoteServer::onSecurityRequest() {
    Serial.println("[BLE Security] Peer requested security pairing.");
    return true;
}

void BLEHidRemoteServer::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (desc->sec_state.bonded) {
        Serial.println("[BLE Security] Authentication Successful! Device Bonded.");
    } else {
        Serial.println("[BLE Security] Authentication Completed (not bonded).");
    }
}

bool BLEHidRemoteServer::onConfirmPIN(uint32_t pin) {
    Serial.printf("[BLE Security] Confirm PIN: %06u\n", pin);
    return true;
}

void BLEHidRemoteServer::sendKey(uint8_t hidPage, uint16_t hidCode) {
    if (!connected) {
        Serial.printf("[BLE] Cannot send key 0x%04X (Page 0x%02X) - Not connected\n", hidCode, hidPage);
        return;
    }

    if (hidPage == 0x07) {
        uint8_t reportOn[8] = {0, 0, (uint8_t)hidCode, 0, 0, 0, 0, 0};
        uint8_t reportOff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        
        inputKeyboard->setValue(reportOn, sizeof(reportOn));
        inputKeyboard->notify();
        delay(5);
        inputKeyboard->setValue(reportOff, sizeof(reportOff));
        inputKeyboard->notify();
        Serial.printf("[BLE] Sent Keyboard Scancode: 0x%02X\n", hidCode);
    } else if (hidPage == 0x0C) {
        uint8_t reportOn[2] = { (uint8_t)(hidCode & 0xFF), (uint8_t)((hidCode >> 8) & 0xFF) };
        uint8_t reportOff[2] = { 0, 0 };

        inputConsumer->setValue(reportOn, sizeof(reportOn));
        inputConsumer->notify();
        delay(5);
        inputConsumer->setValue(reportOff, sizeof(reportOff));
        inputConsumer->notify();
        Serial.printf("[BLE] Sent Consumer Control Usage: 0x%04X\n", hidCode);
    }
}
