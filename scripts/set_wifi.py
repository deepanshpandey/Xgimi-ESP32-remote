#!/usr/bin/env python3
"""
set_wifi.py
Configures Home Wi-Fi credentials on the ESP32 over USB Serial (COM3)
Usage:
    py scripts/set_wifi.py "MyWiFiSSID" "MyWiFiPassword" [COM3]
"""

import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Error: pyserial is required. Install via: pip install pyserial")
    sys.exit(1)

def find_esp32_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        hwid = p.hwid.upper() if p.hwid else ""
        desc = p.description.upper() if p.description else ""
        if any(keyword in hwid or keyword in desc for keyword in ["1A86", "CP210", "CH340", "USB-SERIAL", "ESP32", "USB JTAG"]):
            return p.device
    if ports:
        return ports[0].device
    if sys.platform.startswith("win"):
        return "COM3"
    elif sys.platform.startswith("darwin"):
        return "/dev/cu.usbserial-0001"
    else:
        return "/dev/ttyUSB0"

def main():
    if len(sys.argv) < 3:
        cmd_name = "python" if sys.platform.startswith("win") else "python3"
        print(f"Usage: {cmd_name} scripts/set_wifi.py <SSID> <PASSWORD> [PORT]")
        print(f"Example (Windows): {cmd_name} scripts/set_wifi.py \"MyHomeNetwork\" \"Secret12345\" COM3")
        print(f"Example (macOS):   python3 scripts/set_wifi.py \"MyHomeNetwork\" \"Secret12345\" /dev/cu.usbserial-0001")
        print(f"Example (Linux):   python3 scripts/set_wifi.py \"MyHomeNetwork\" \"Secret12345\" /dev/ttyUSB0")
        sys.exit(1)

    ssid = sys.argv[1].strip()
    password = sys.argv[2].strip()
    port = sys.argv[3].strip() if len(sys.argv) > 3 else find_esp32_port()

    print(f"Connecting to ESP32 on {port}...")
    try:
        ser = serial.Serial(port, 115200, timeout=1)
    except Exception as e:
        print(f"Failed to open serial port {port}: {e}")
        sys.exit(1)

    time.sleep(0.5)
    ser.reset_input_buffer()

    # Command format recognized by ESP32: SET_WIFI:ssid,password\n
    cmd = f"SET_WIFI:{ssid},{password}\n"
    print(f"Sending Wi-Fi credentials for SSID '{ssid}' over USB...")
    ser.write(cmd.encode("utf-8"))

    start = time.time()
    while time.time() - start < 15:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print(f"[ESP32] {line}")
            if "Connected to Home Wi-Fi" in line or "Web Remote URL" in line:
                break
        time.sleep(0.1)

    ser.close()
    print("Done!")

if __name__ == "__main__":
    main()
