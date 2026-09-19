# 🎛️ XGIMI ESP32 BLE Remote & Web Controller

A full-featured ESP32 remote controller that emulates the official **XGIMI Bluetooth Remote Control** (`Vendor_000d_Product_3838.kl`) via Bluetooth Low Energy (BLE) HID and serves a web application interface over Wi-Fi.

---

## 🌟 Key Features

- **XGIMI BLE HID Emulation**: Identifies as Vendor `0x000D` / Product `0x3838` (Name: `XGIMIRC`). The XGIMI projector's Android OS automatically matches it to `/vendor/usr/keylayout/Vendor_000d_Product_3838.kl`.
- **Dynamic `.kl` Parser**: Automated Python tooling reads `Vendor_000d_Product_3838.kl` and builds C++ HID scancode headers.
- **Embedded Web Remote App**: Serves a sleek, glassmorphic dark-mode remote control directly from the ESP32 over Wi-Fi.
- **All Special XGIMI Keys Supported**:
  - 🔍 **Autofocus** (`FOCUS_AUTO` / F11 / Linux key 87)
  - 🎯 **Manual Focus** (`FOCUS_MAN_NEW` / F4 / Linux key 62)
  - ⬅️ **Focus Left** (`FOCUS_LEFT` / F9 / Linux key 67)
  - ➡️ **Focus Right** (`FOCUS_RIGHT` / F10 / Linux key 68)
  - ⚙️ **Projector Settings** (`XGIMI_MISCKEY` / F8 / Linux key 66)
  - 🔌 **Source Input** (`XGIMI_SOURCE` / F7 / Linux key 65)
  - 🎙️ **Voice Assistant** (`VOICE_ASSIST` / Search / Linux key 217)
  - **Full D-Pad** (Up, Down, Left, Right, OK/Select)
  - **Power & Audio Controls** (Power, Vol+, Vol-, Mute)

---

## 📂 File Structure

```text
Xgimi-ESP32-remote/
├── Vendor_000d_Product_3838.kl   # Official XGIMI Key Layout file
├── platformio.ini                 # PlatformIO configuration & dependencies
├── scripts/
│   ├── generate_keymap.py         # Parses .kl file to produce include/xgimi_keymap.h
│   ├── bundle_web.py              # Inlines web UI into include/web_assets.h
│   └── build_hooks.py             # Pre-build script for PlatformIO
├── include/
│   ├── xgimi_keymap.h             # Auto-generated scancode definitions
│   └── web_assets.h               # Auto-generated embedded Web UI (PROGMEM)
├── src/
│   ├── main.cpp                   # System entry point
│   ├── ble_hid_remote.h/.cpp      # NimBLE HID server (VID: 0x000D, PID: 0x3838)
│   └── web_server.h/.cpp          # Web Server & WebSocket/REST API endpoints
└── web/
    ├── index.html                 # Remote Web UI structure
    ├── style.css                  # Glassmorphism & dark-mode styling
    └── app.js                     # Tactile feedback & WebSocket communication
```

---

## 🚀 Quick Start Guide

### 1. Build & Upload Firmware

Using **PlatformIO**:
```bash
# Build firmware
pio run

# Flash to ESP32 board
pio run --target upload

# Open Serial Monitor
pio device monitor
```

### 2. Connect Your Phone / Web Browser

1. Power on the ESP32.
2. On your smartphone or laptop, connect to the Wi-Fi network:
   - **SSID**: `XGIMI-Remote-AP`
   - **Password**: `xgimiremote`
3. Open browser and go to:
   ```text
   http://192.168.4.1
   ```

### 3. Pair ESP32 with XGIMI Projector

1. Turn on your XGIMI Projector.
2. Go to **Settings** ⚙️ ➔ **Remotes & Accessories** ➔ **Add accessory**.
3. Select **`XGIMIRC`** from the list of available Bluetooth devices.
4. Once paired, pressing any button on the Web Remote will immediately trigger the corresponding XGIMI action!

---

## 📡 REST API Endpoint

You can also send remote commands programmatically:

```bash
# Trigger Autofocus
curl -X POST "http://192.168.4.1/api/press?action=FOCUS_AUTO"

# Trigger Projector Settings
curl -X POST "http://192.168.4.1/api/press?action=XGIMI_MISCKEY"

# Trigger D-Pad Up
curl -X POST "http://192.168.4.1/api/press?action=DPAD_UP"
```
