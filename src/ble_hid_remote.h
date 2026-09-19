#ifndef BLE_HID_REMOTE_H
#define BLE_HID_REMOTE_H

#include <Arduino.h>
#include <NimBLEDevice.h>

class BLEHidRemoteServer : public NimBLEServerCallbacks, public NimBLESecurityCallbacks {
private:
    NimBLEHIDDevice* hid;
    NimBLECharacteristic* inputKeyboard;
    NimBLECharacteristic* inputConsumer;
    bool connected;
    bool advertising;
    uint16_t currentConnId;

public:
    BLEHidRemoteServer();
    void begin(const char* deviceName = "XGIMI-RC-pseudo", uint16_t vid = 0x000D, uint16_t pid = 0x3838);
    
    bool isConnected() const { return connected; }
    bool isAdvertising() const { return advertising; }
    int getBondedCount() const;
    String getConnectedAddress() const;

    void startPairingMode(uint32_t durationSeconds = 60);
    void stopPairingMode();
    void clearBonds();
    void sendKey(uint8_t hidPage, uint16_t hidCode);
    
    // NimBLEServerCallbacks overrides
    void onConnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer) override;

    // NimBLESecurityCallbacks overrides
    uint32_t onPassKeyRequest() override;
    void onPassKeyNotify(uint32_t pass_key) override;
    bool onSecurityRequest() override;
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override;
    bool onConfirmPIN(uint32_t pin) override;
};

extern BLEHidRemoteServer BleRemote;

#endif // BLE_HID_REMOTE_H
