# M5 Cardputer Deauth Detector

A handheld Wi-Fi security monitor built for the M5Stack Cardputer (StampS3) that detects deauthentication attacks on protected networks.

## Features

- **Real-time Deauth Detection**: Monitors for deauthentication packets targeting specified SSIDs
- **Hardware Feedback**: Immediate buzzer and LED alerts when attacks are detected
- **Local Logging**: Saves all events to SD card in CSV format
- **Remote API Reporting**: Sends batch alerts to a configurable REST API endpoint
- **Web Configuration Portal**: Easy setup via browser interface
- **Multiple Display Views**: Dashboard, live log, and detailed statistics

## Hardware Requirements

- M5Stack Cardputer (ESP32-S3)
- MicroSD card (for configuration and logging)
- Built-in display, keyboard, buzzer, and LED

## Installation

### Using PlatformIO

1. Clone or download this project
2. Open in VS Code with PlatformIO extension
3. Connect your M5 Cardputer via USB
4. Build and upload:
   ```
   pio run --target upload
   ```

### Initial Setup

1. Insert a formatted microSD card into the M5 Cardputer
2. Copy `config.txt.example` to the SD card root and rename it to `config.txt`
3. Edit `config.txt` with your settings (or use the web interface after first boot)
4. Power on the device

## Configuration

### First Boot

If no valid configuration is found, the device will automatically enter **Config Mode**:

1. Device creates WiFi AP: `M5-DeauthDetector`
2. Connect to this network from your phone/computer
3. Navigate to `http://192.168.4.1`
4. Login with default credentials:
   - Username: `admin`
   - Password: `1234`
5. Configure settings and save

### Configuration File Structure

The `config.txt` file uses JSON format with the following sections:

#### WiFi Settings
```json
"wifi": {
  "sta_ssid": "Your_Home_Network",
  "sta_password": "Your_Password",
  "admin_user": "admin",
  "admin_pass": "1234"
}
```

#### NTP Settings
```json
"ntp": {
  "server": "pool.ntp.org",
  "timezone_offset": 0,
  "daylight_savings": false
}
```

#### Detection Settings
```json
"detection": {
  "protected_ssids": ["Home_WiFi", "Office_Secure"],
  "silence_gap_seconds": 30,
  "led_hold_seconds": 300,
  "reporting_interval_seconds": 10
}
```

#### API Settings
```json
"api": {
  "endpoint_url": "https://your-api.com/v1/alerts",
  "custom_header_name": "X-API-KEY",
  "custom_header_value": "secret-token-123"
}
```

#### Hardware Settings
```json
"hardware": {
  "buzzer_freq": 2000,
  "buzzer_duration_ms": 2000,
  "screen_brightness": 128
}
```

## Usage

### Monitor Mode (Normal Operation)

1. Device scans for protected SSIDs and identifies their channels
2. Enters promiscuous mode to monitor for deauth packets
3. When attack detected:
   - Buzzer sounds for configured duration
   - LED turns solid red
   - Event logged to SD card
   - Event queued for API reporting

### Display Views

Press **ENTER** to cycle through views:

1. **Dashboard**: Shows all protected SSIDs with attack counters
2. **Live Log**: Displays the 5 most recent events
3. **Detailed**: One SSID per page with full statistics

### Entering Config Mode

Hold the **GO button** (G0) for 2 seconds while in Monitor Mode

### API Payload Format

Events are sent as JSON array via POST:

```json
[
  {
    "timestamp": "2026-01-13T10:15:00Z",
    "target_ssid": "Home_WiFi",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -45,
    "packet_count": 24
  }
]
```

### Log Files

Logs are stored in `/logs/` directory on SD card:
- Format: CSV
- Filename: `session_YYYYMMDD_HHMMSS.csv`
- New session file created on each boot

## Troubleshooting

### SD Card Error
- Ensure SD card is formatted as FAT32
- Check that card is properly inserted
- Try a different SD card

### Config Not Loading
- Verify `config.txt` exists in SD card root
- Check JSON syntax is valid
- Review serial monitor output for error messages

### No WiFi Connection
- Verify SSID and password in config
- Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Ensure network is within range

### No Deauth Detection
- Verify protected SSIDs are broadcasting
- Check that SSIDs are spelled correctly in config
- Ensure networks are on 2.4GHz band

## Technical Notes

### Channel Hopping
The device only monitors channels where protected SSIDs are detected, optimizing detection efficiency.

### Promiscuous Mode
WiFi adapter operates in promiscuous mode to capture all management frames on monitored channels.

### Time Synchronization
Device syncs time via NTP on boot. If sync fails, timestamps will be incorrect until next successful sync.

## Development

### Project Structure
```
deauth_detector/
├── include/              # Header files
│   ├── Config.h
│   ├── ConfigManager.h
│   ├── DeauthDetector.h
│   ├── Display.h
│   ├── WiFiManager.h
│   ├── WebPortal.h
│   ├── Logger.h
│   ├── APIReporter.h
│   └── AlertManager.h
├── src/                  # Implementation files
│   ├── main.cpp
│   ├── ConfigManager.cpp
│   ├── DeauthDetector.cpp
│   ├── Display.cpp
│   ├── WiFiManager.cpp
│   ├── WebPortal.cpp
│   ├── Logger.cpp
│   ├── APIReporter.cpp
│   └── AlertManager.cpp
├── platformio.ini        # PlatformIO configuration
└── config.txt.example    # Example configuration
```

### Dependencies
- M5Cardputer library
- ArduinoJson
- ESP32 WiFi (built-in)
- SD (built-in)

## License

This project is provided as-is for educational and security research purposes.

## Disclaimer

This tool is intended for monitoring your own networks and authorized security testing only. Unauthorized interception of wireless communications may be illegal in your jurisdiction. Use responsibly.
