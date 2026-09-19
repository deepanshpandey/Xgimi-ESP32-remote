# 🎛️ XGIMI ESP32 BLE Remote & Web Controller

A full-featured ESP32 remote controller that emulates the official **XGIMI Bluetooth Remote Control** (`Vendor_000d_Product_3838.kl`) via Bluetooth Low Energy (BLE) HID and serves a web application interface over Wi-Fi.

---

## 🌟 Key Features

- **XGIMI BLE HID Emulation**: Identifies as Vendor `0x000D` / Product `0x3838` (Name: `XGIMI-RC-pseudo`). The XGIMI projector's Android OS automatically matches it to `/vendor/usr/keylayout/Vendor_000d_Product_3838.kl`.
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
│   ├── build_hooks.py             # Pre-build script for PlatformIO
│   └── set_wifi.py                # Sets Wi-Fi credentials over USB Serial
├── include/
│   ├── wifi_config.h              # Home Wi-Fi configuration header
│   ├── xgimi_keymap.h             # Auto-generated scancode definitions
│   └── web_assets.h               # Auto-generated embedded Web UI (PROGMEM)
├── src/
│   ├── main.cpp                   # System entry point
│   ├── wifi_manager.h/.cpp        # Wi-Fi STA, SoftAP, NVS storage & mDNS
│   ├── ble_hid_remote.h/.cpp      # NimBLE HID server (VID: 0x000D, PID: 0x3838)
│   └── web_server.h/.cpp          # Web Server & WebSocket/REST API endpoints
└── web/
    ├── index.html                 # Remote Web UI structure & Pairing Modal
    ├── style.css                  # Glassmorphism & dark-mode styling
    └── app.js                     # Tactile feedback, Pairing & Wi-Fi logic
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

# Open Serial Monitor (115200 baud)
pio device monitor
```

*(Note: If `pio` is not in your global PATH, invoke via your PlatformIO environment: on Windows `~/.platformio/penv/Scripts/platformio.exe run --target upload`, or on macOS/Linux `~/.platformio/penv/bin/platformio run --target upload`)*

### 2. Home Wi-Fi Setup (3 Easy Methods)

You can connect the ESP32 to your home Wi-Fi network so you don't have to switch Wi-Fi networks on your phone:

#### Method A: Via Configuration File (`include/wifi_config.h`)
Edit [`include/wifi_config.h`](include/wifi_config.h) before uploading:
```cpp
#define HOME_WIFI_SSID     "YourHomeWiFi"
#define HOME_WIFI_PASSWORD "YourPassword"
```

#### Method B: Instantly via USB Serial (No Re-flashing Needed)
Ensure `pyserial` is installed (`pip install pyserial`), connect the ESP32 via USB, and run:

**On Windows:**
```bash
python scripts/set_wifi.py "YourHomeWiFi" "YourPassword"
```
*(If multiple COM ports exist, you can optionally specify the port: `python scripts/set_wifi.py "YourHomeWiFi" "YourPassword" COM3`)*

**On macOS / Linux:**
```bash
python3 scripts/set_wifi.py "YourHomeWiFi" "YourPassword"
```
*(Optionally specify device path, e.g. `/dev/cu.usbserial-0001` or `/dev/ttyUSB0`)*

The credentials are saved directly into the ESP32's non-volatile flash memory (NVS) and it will immediately connect to your network.

#### Method C: Through the WebApp Interface
1. Connect to the ESP32's direct Wi-Fi hotspot:
   - **SSID**: `XGIMI-Remote-AP`
   - **Password**: `xgimiremote`
2. Open **`http://192.168.4.1`** in your browser.
3. Tap the **`[ 🔗 ]`** button in the header.
4. Under **"CONNECT TO HOME WI-FI"**, enter your SSID and password, then tap **"Connect & Save"**.

---

### 3. Accessing the Web Remote

- **On Home Wi-Fi**: Open **`http://xgimi-remote.local`** (or the IP assigned by your router).
- **On Direct Hotspot**: Open **`http://192.168.4.1`** (connected to `XGIMI-Remote-AP`).

---

### 4. Pair ESP32 with XGIMI Projector

1. Turn on your XGIMI Projector.
2. Go to **Settings** ⚙️ ➔ **Remotes & Accessories** ➔ **Add accessory**.
3. Select **`XGIMI-RC-pseudo`** from the list of available Bluetooth devices.
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

---

## ⚖️ Disclaimer & Legal Notice

The key layout and "XGIMI" trademark are the property of **Chengdu XGIMI Technology Co., Ltd.** (often referred to simply as **XGIMI Technology Co., Ltd.**). This project is an independent open-source controller intended for personal interoperability and educational use.

