# 🎛️ XGIMI ESP32 BLE Remote & Web Controller

A high-performance ESP32 remote controller that emulates the official **XGIMI Bluetooth Remote Control** (`Vendor_000d_Product_3838.kl`) via Bluetooth Low Energy (BLE) HID and serves an ultra-responsive, pitch-black Google TV styled web remote over Wi-Fi.

---

## 🌟 Key Features

- **XGIMI BLE HID Emulation**: Broadcasts as **`XGIMI RC pseudo`** (Vendor ID `0x000D`, Product ID `0x3838`). The projector's Android TV OS matches it to `/vendor/usr/keylayout/Vendor_000d_Product_3838.kl`.
- **Official Remote Recognition & Battery Reporting**: Exposes Bluetooth SIG GAP Appearance `0x0180` (Generic Remote Control) and GATT Battery Service (`0x180F`) reporting **100% Battery**, allowing Android TV to classify it as a native remote with a dedicated Remote icon.
- **Ultra-Fast, Low-Latency Response**:
  - Negotiates fast **7.5ms – 15ms BLE connection intervals** upon connecting to the projector.
  - Zero-overhead raw WebSocket action dispatch (avoids JSON serialization latency).
  - High-frequency 1ms event loop servicing on the ESP32.
  - Instantaneous `pointerdown` actuation (no 300ms mobile touch delay).
- **Pitch-Black Google TV Remote Interface**:
  - Full-screen responsive layout utilizing modern OLED black tones (`#000000`).
  - Scoped mouse-only hover (`@media (hover: hover) and (pointer: fine)`) and instant focus clearance (`button.blur()`) to eliminate sticky silhouettes on touchscreens.
  - Symmetrical header: Power, Autofocus (`FOCUS_AUTO`), Settings (`XGIMI_MISCKEY`), and Source Input (`XGIMI_SOURCE`).
  - Circular navigation D-Pad (Up, Down, Left, Right, OK/Center).
  - Android function row: Back, Home, Menu, and Voice Assistant.
  - Symmetrical 2-button Volume Bar (`VOL -` and `VOL +`).
- **Tactile Haptic Vibration Engine**: Uses `navigator.vibrate` to provide distinct haptic feedback patterns (navigation clicks, firm OK pulse, double-pulse power confirmation).
- **Dual-Mode Wi-Fi with Zero Radio Conflict**: Direct SoftAP (`192.168.4.1`) and Home Wi-Fi (`xgimi-remote.local`), dynamically synchronizing RF channels to eliminate packet loss and connection errors.
- **Installable Progressive Web App (PWA)**: Add to iOS or Android home screens for a full-screen, native remote experience without browser chrome.

---

## 📂 File Structure

```text
Xgimi-ESP32-remote/
├── Vendor_000d_Product_3838.kl   # Official XGIMI Key Layout file
├── platformio.ini                 # PlatformIO build configuration & libraries
├── scripts/
│   ├── generate_keymap.py         # Parses .kl to produce include/xgimi_keymap.h
│   ├── bundle_web.py              # Compresses & inlines web assets into web_assets.h
│   ├── build_hooks.py             # Pre-build automation hook for PlatformIO
│   └── set_wifi.py                # Configures Wi-Fi credentials over USB Serial (COM3)
├── include/
│   ├── wifi_config.h              # Compile-time Home Wi-Fi configuration
│   ├── xgimi_keymap.h             # Auto-generated scancode definitions & device name
│   └── web_assets.h               # Embedded zero-copy Web UI assets (PROGMEM)
├── src/
│   ├── main.cpp                   # Main setup & 1ms loop scheduler
│   ├── wifi_manager.h/.cpp        # STA, SoftAP, channel alignment & NVS storage
│   ├── ble_hid_remote.h/.cpp      # NimBLE HID server with low-latency parameters
│   └── web_server.h/.cpp          # High-speed WebSocket server & REST API
└── web/
    ├── index.html                 # Remote Web UI structure & Pairing Modal
    ├── style.css                  # Pitch-black Google TV CSS & responsive layout
    ├── app.js                     # WebSocket, haptic engine, & instantaneous input logic
    ├── manifest.json              # PWA installation manifest
    ├── icon.svg                   # Minimalist vector remote icon
    └── sw.js                      # Offline cache service worker
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

*(Note: On Windows using the PlatformIO penv: `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --target upload`)*

---

### 2. Home Wi-Fi Setup (3 Easy Methods)

Connect the ESP32 to your home Wi-Fi network so you can control it from any phone or computer on the same network:

#### Method A: Via Configuration File (`include/wifi_config.h`)
Edit [`include/wifi_config.h`](include/wifi_config.h) before building:
```cpp
#define HOME_WIFI_SSID     "YourHomeWiFi"
#define HOME_WIFI_PASSWORD "YourPassword"
```

#### Method B: Instantly via USB Serial (No Re-flashing Required)
Run the helper script while the ESP32 is plugged into USB:

**On Windows:**
```bash
python scripts/set_wifi.py "YourHomeWiFi" "YourPassword"
```
*(Optionally specify COM port: `python scripts/set_wifi.py "YourHomeWiFi" "YourPassword" COM3`)*

**On macOS / Linux:**
```bash
python3 scripts/set_wifi.py "YourHomeWiFi" "YourPassword"
```

Credentials are saved permanently into the ESP32's non-volatile storage (NVS) and it will connect instantly.

#### Method C: Through the WebApp Interface
1. Connect to the ESP32's Wi-Fi hotspot:
   - **SSID**: `XGIMI-Remote-AP`
   - **Password**: `xgimiremote`
2. Open **`http://192.168.4.1`** in your browser.
3. Tap the **`[ 🔗 ]`** button in the header.
4. Under **"CONNECT TO HOME WI-FI"**, enter your SSID and password, then tap **"Connect & Save"**.

---

### 3. Accessing the Web Remote

- **On Home Wi-Fi**: Open **`http://xgimi-remote.local`** (or the local IP assigned by your router, e.g. `http://192.168.68.109`).
- **On Direct Hotspot**: Open **`http://192.168.4.1`** (connected to `XGIMI-Remote-AP`).

---

### 4. Pair with Your XGIMI Projector

1. Turn on your XGIMI Projector.
2. Navigate to **Settings** ⚙️ ➔ **Remotes & Accessories** ➔ **Add accessory**.
3. Select **`XGIMI RC pseudo`** from the list.
4. If you had previously paired the ESP32 under an older name, click on the old entry and choose **Unpair / Forget** first, then re-pair.
5. Once paired, the projector will show **`XGIMI RC pseudo`** with the official **Remote Control icon** and **`Battery 100%`**!

---

## 📡 REST API Endpoint

You can also trigger remote actions programmatically via HTTP requests:

```bash
# Trigger Autofocus
curl -X POST "http://192.168.68.109/api/press?action=FOCUS_AUTO"

# Trigger Projector Settings
curl -X POST "http://192.168.68.109/api/press?action=XGIMI_MISCKEY"

# Trigger D-Pad Up
curl -X POST "http://192.168.68.109/api/press?action=DPAD_UP"

# Trigger Volume Up
curl -X POST "http://192.168.68.109/api/press?action=VOLUME_UP"
```

---

## ⚖️ Disclaimer & Legal Notice

The key layout and "XGIMI" trademark are the property of **Chengdu XGIMI Technology Co., Ltd.**. This project is an independent open-source controller intended for personal interoperability and educational use.
