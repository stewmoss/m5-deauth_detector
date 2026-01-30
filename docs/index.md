# M5 Cardputer Deauth Detector Documentation

Welcome to the complete documentation for the M5 Cardputer Deauth Detector—a portable Wi-Fi security monitoring device that detects 802.11 deauthentication attacks in real-time.

## Table of Contents

1. [Getting Started](getting-started.md) — Installation, hardware setup, and first boot
2. [Configuration Reference](configuration.md) — Complete configuration file documentation
3. [Web Interface Guide](web-interface.md) — Using the browser-based configuration portal
4. [Operation Guide](operation.md) — Display views, alerts, and daily usage
5. [API Integration](api-integration.md) — REST API payload format and integration
6. [Troubleshooting](troubleshooting.md) — Common issues and solutions

---

## What is a Deauthentication Attack?

A deauthentication attack is a type of denial-of-service (DoS) attack that targets the IEEE 802.11 (Wi-Fi) protocol. Attackers send forged deauthentication frames to wireless clients, forcing them to disconnect from their access point.

**Common attack scenarios include:**

- **Evil Twin Attacks** — Forcing clients off a legitimate network to connect to a rogue access point
- **Network Disruption** — Continuous deauthentication to deny service
- **Handshake Capture** — Forcing reconnection to capture WPA handshakes for offline cracking
- **Security Testing** — Penetration testers may use deauth attacks during authorized assessments

This device monitors your protected networks and immediately alerts you when such attacks are detected.

---

## How the Detector Works

### Detection Methodology

1. **Channel Discovery** — On startup, the device scans all 14 Wi-Fi channels (2.4 GHz) to locate your protected SSIDs
2. **Targeted Monitoring** — The device then monitors only the channels where your networks are broadcasting
3. **Promiscuous Mode** — The ESP32-S3 Wi-Fi adapter enters promiscuous mode to capture all 802.11 management frames
4. **Frame Analysis** — Each captured frame is analyzed; deauthentication frames (subtype 0x0C) trigger alerts
5. **Attack Attribution** — The device logs the attacker MAC address, target BSSID, channel, and signal strength

### Alert System

When an attack is detected:

| Response | Description |
|----------|-------------|
| **Buzzer** | Audible alert sounds for a configurable duration |
| **LED** | SK6812 RGB LED turns solid red |
| **Display** | Attack counter increments on the dashboard |
| **Log** | Event is appended to the SD card CSV log |
| **API** | Event is queued for batch reporting to your endpoint |

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        M5 Cardputer                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐ │
│  │   Display   │    │  Detector   │    │     WiFi Manager    │ │
│  │   Manager   │    │   Engine    │    │  (STA/AP/Promis.)   │ │
│  └──────┬──────┘    └──────┬──────┘    └──────────┬──────────┘ │
│         │                  │                       │            │
│         └──────────────────┼───────────────────────┘            │
│                            │                                    │
│  ┌─────────────┐    ┌──────┴──────┐    ┌─────────────────────┐ │
│  │   Alert     │    │    Main     │    │      Config         │ │
│  │   Manager   │◄───┤ Controller  ├───►│      Manager        │ │
│  └─────────────┘    └──────┬──────┘    └─────────────────────┘ │
│                            │                                    │
│  ┌─────────────┐    ┌──────┴──────┐    ┌─────────────────────┐ │
│  │   Logger    │    │  Web Portal │    │    API Reporter     │ │
│  │   (SD Card) │    │  (HTTP/AP)  │    │    (REST POST)      │ │
│  └─────────────┘    └─────────────┘    └─────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Operational Modes

### 1. Configuration Mode (AP Mode)

The device hosts a Wi-Fi access point and web server for configuration.

**Entry conditions:**
- First boot with no valid configuration
- Manual entry by holding the G0 button for 2 seconds
- Web portal timeout (5 minutes of inactivity returns to monitoring)

**Access point details:**
- SSID: `M5-DeauthDetector`
- IP Address: `192.168.4.1`
- Protected by HTTP Basic Authentication

### 2. Monitor Mode (Normal Operation)

The device actively monitors for deauthentication attacks.

**Startup sequence:**
1. Load configuration from SD card
2. Connect to configured WiFi (STA mode)
3. Synchronize time via NTP
4. Disconnect from WiFi
5. Discover protected SSID channels
6. Enter promiscuous mode and begin monitoring

**During monitoring:**
- Channel hopping across active channels
- Real-time packet analysis
- Periodic WiFi reconnection for API batch uploads

---

## Hardware Overview

### M5Stack Cardputer Specifications

| Component | Details |
|-----------|---------|
| **MCU** | ESP32-S3 (dual-core Xtensa LX7, 240 MHz) |
| **Display** | 1.14" 240×135 TFT LCD (ST7789) |
| **Input** | Full QWERTY keyboard + G0 button |
| **Storage** | MicroSD card slot (SPI interface) |
| **LED** | SK6812 addressable RGB LED |
| **Audio** | Built-in buzzer (GPIO 2) |
| **Connectivity** | Wi-Fi 802.11 b/g/n (2.4 GHz only) |

### Pin Assignments

| Function | GPIO Pin |
|----------|----------|
| SD Card SCK | GPIO 40 |
| SD Card MISO | GPIO 39 |
| SD Card MOSI | GPIO 14 |
| SD Card CS | GPIO 12 |
| Buzzer | GPIO 2 |
| RGB LED | GPIO 21 |

---

## File System Structure

The device creates and manages the following structure on the SD card:

```
SD Card Root
├── config.txt                              # Device configuration (JSON)
└── deauthdetector/
    └── logs/
        ├── deauthdetect_session_*.csv      # Session logs (one per boot)
        └── debug.log                       # Debug output (if enabled)
```

---

## Quick Reference

| Action | Method |
|--------|--------|
| Enter config mode | Hold G0 button for 2 seconds |
| Cycle display views | Press ENTER key |
| Access web portal | Connect to AP, browse to 192.168.4.1 |
| Default credentials | admin / 1234 |
| View debug log | Web portal → Debug tab → View Debug Log |
| Clear debug log | Web portal → Debug tab → Clear Debug Log |

---

## Next Steps

→ [Getting Started](getting-started.md) — Begin installation and setup
