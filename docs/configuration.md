# Configuration Reference

This document provides a complete reference for the `config.txt` configuration file, including all available options, their data types, default values, and usage examples.

---

## Overview

The configuration is stored in JSON format in a file named `config.txt` located at the root of the SD card. The device reads this file on each boot.

### File Location

```
SD Card Root
└── config.txt
```

### Configuration Methods

Configuration can be modified through:

1. **Web Interface** — Recommended for most users
2. **Direct File Editing** — Edit `config.txt` on a computer
3. **Pre-deployment** — Use `config.txt.example` as a template

---

## Complete Configuration Structure

```json
{
  "wifi": {
    "sta_ssid": "Your_Home_Network",
    "sta_password": "Your_Password",
    "admin_user": "admin",
    "admin_pass": "1234"
  },
  "ntp": {
    "server": "pool.ntp.org",
    "timezone_offset": 0,
    "daylight_savings": false
  },
  "detection": {
    "protected_ssids": ["Home_WiFi", "Office_Secure"],
    "silence_gap_seconds": 30,
    "led_hold_seconds": 300,
    "reporting_interval_seconds": 10
  },
  "api": {
    "endpoint_url": "https://your-api.com/v1/alerts",
    "custom_header_name": "X-API-KEY",
    "custom_header_value": "secret-token-123"
  },
  "hardware": {
    "buzzer_freq": 2000,
    "buzzer_duration_ms": 2000,
    "screen_brightness": 128,
    "fancy_intro": true
  },
  "debug": {
    "enabled": false
  }
}
```

---

## Configuration Sections

### WiFi Configuration (`wifi`)

Controls WiFi connectivity and web portal authentication.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `sta_ssid` | String | `""` | SSID of the WiFi network to connect to for NTP sync and API reporting |
| `sta_password` | String | `""` | Password for the WiFi network |
| `admin_user` | String | `"admin"` | Username for web portal authentication |
| `admin_pass` | String | `"1234"` | Password for web portal authentication |

**Example:**

```json
"wifi": {
  "sta_ssid": "MyHomeNetwork",
  "sta_password": "MySecurePassword123",
  "admin_user": "detector",
  "admin_pass": "StrongPassword456"
}
```

> ⚠️ **Security Note:** Change the default admin credentials before deploying the device.

---

### NTP Configuration (`ntp`)

Controls time synchronization for accurate event timestamps.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `server` | String | `"pool.ntp.org"` | NTP server hostname or IP address |
| `timezone_offset` | Integer | `0` | UTC offset in hours (e.g., `-5` for EST, `+10` for AEST) |
| `daylight_savings` | Boolean | `false` | Whether to add 1 hour for daylight savings time |

**Example:**

```json
"ntp": {
  "server": "time.google.com",
  "timezone_offset": -5,
  "daylight_savings": true
}
```

**Common Timezone Offsets:**

| Timezone | Offset |
|----------|--------|
| UTC | 0 |
| Eastern (EST) | -5 |
| Central (CST) | -6 |
| Pacific (PST) | -8 |
| UK (GMT) | 0 |
| Central Europe (CET) | +1 |
| Australia East (AEST) | +10 |

---

### Detection Configuration (`detection`)

Controls which networks to monitor and alert behavior.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `protected_ssids` | String[] | `[]` | Array of SSID names to monitor for attacks |
| `silence_gap_seconds` | Integer | `30` | Seconds of silence before starting LED countdown |
| `led_hold_seconds` | Integer | `300` | Seconds to keep LED red after silence gap (5 minutes) |
| `reporting_interval_seconds` | Integer | `10` | Interval for batch API reporting |

**Example:**

```json
"detection": {
  "protected_ssids": [
    "Home_WiFi",
    "Home_WiFi_5G",
    "Office_Secure",
    "Guest_Network"
  ],
  "silence_gap_seconds": 60,
  "led_hold_seconds": 600,
  "reporting_interval_seconds": 30
}
```

#### Understanding Alert Timing

The LED and alert system uses a two-phase approach:

1. **Active Phase:** LED is solid red while deauth packets are being received
2. **Silence Gap:** After `silence_gap_seconds` with no packets, the countdown begins
3. **LED Hold:** LED remains red for `led_hold_seconds` after the silence gap
4. **Clear:** LED turns off after the hold period expires

```
Attack Detected          Silence Gap           LED Hold         Clear
     │                       │                    │               │
     ▼                       ▼                    ▼               ▼
    ┌───────────────────────┬───────────────────┬───────────────┐
    │   LED RED (active)    │  LED RED (hold)   │   LED OFF     │
    └───────────────────────┴───────────────────┴───────────────┘
    │◄─ Packets arriving ──►│◄── 30 seconds ───►│◄─ 5 minutes ─►│
```

---

### API Configuration (`api`)

Controls remote reporting to external security systems.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `endpoint_url` | String | `""` | Full URL for POST requests (HTTP or HTTPS) |
| `custom_header_name` | String | `""` | Custom HTTP header name for authentication |
| `custom_header_value` | String | `""` | Value for the custom header |

**Example:**

```json
"api": {
  "endpoint_url": "https://security.example.com/api/v1/deauth-alerts",
  "custom_header_name": "Authorization",
  "custom_header_value": "Bearer eyJhbGciOiJIUzI1NiIs..."
}
```

#### Supported Authentication Methods

| Method | Header Name | Header Value |
|--------|-------------|--------------|
| API Key | `X-API-KEY` | `your-api-key` |
| Bearer Token | `Authorization` | `Bearer your-token` |
| Basic Auth | `Authorization` | `Basic base64-encoded` |
| Custom | Any header name | Any value |

#### Disabling API Reporting

To disable API reporting, leave the `endpoint_url` empty:

```json
"api": {
  "endpoint_url": "",
  "custom_header_name": "",
  "custom_header_value": ""
}
```

For API payload format, see [API Integration](api-integration.md).

---

### Hardware Configuration (`hardware`)

Controls physical device behavior.

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| `buzzer_freq` | Integer | 100-10000 | `2000` | Buzzer frequency in Hz |
| `buzzer_duration_ms` | Integer | 100-30000 | `2000` | How long the buzzer sounds (milliseconds) |
| `screen_brightness` | Integer | 0-255 | `128` | Display brightness level |
| `fancy_intro` | Boolean | — | `true` | Enable animated startup screen |

**Example:**

```json
"hardware": {
  "buzzer_freq": 3000,
  "buzzer_duration_ms": 500,
  "screen_brightness": 200,
  "fancy_intro": false
}
```

#### Buzzer Frequency Guide

| Frequency | Sound |
|-----------|-------|
| 1000 Hz | Low tone |
| 2000 Hz | Medium tone (default) |
| 3000 Hz | High tone |
| 4000 Hz | Very high, attention-grabbing |

#### Brightness Recommendations

| Value | Use Case |
|-------|----------|
| 50-100 | Battery saving, dark environments |
| 128 | Balanced (default) |
| 200-255 | Bright environments, maximum visibility |

---

### Debug Configuration (`debug`)

Controls debug logging for troubleshooting.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `enabled` | Boolean | `false` | Enable/disable debug logging to SD card |

**Example:**

```json
"debug": {
  "enabled": true
}
```

#### Debug Log Details

When enabled:
- Log file: `/deauthdetector/logs/debug.log`
- Contains: WiFi status, config loading, detection events, errors
- Cleared on each device boot
- Can be viewed via the web interface

---

## Example Configurations

### Minimal Configuration

Basic setup for home use with a single network:

```json
{
  "wifi": {
    "sta_ssid": "MyHomeNetwork",
    "sta_password": "MyPassword",
    "admin_user": "admin",
    "admin_pass": "admin123"
  },
  "detection": {
    "protected_ssids": ["MyHomeNetwork"]
  }
}
```

### Enterprise Configuration

Full configuration for security operations:

```json
{
  "wifi": {
    "sta_ssid": "CorpNetwork",
    "sta_password": "SecurePassword",
    "admin_user": "secops",
    "admin_pass": "V3ryS3cur3P@ss!"
  },
  "ntp": {
    "server": "ntp.corp.local",
    "timezone_offset": -5,
    "daylight_savings": true
  },
  "detection": {
    "protected_ssids": [
      "CorpNetwork",
      "CorpNetwork-Guest",
      "CorpNetwork-IoT",
      "Executive-Secure"
    ],
    "silence_gap_seconds": 60,
    "led_hold_seconds": 900,
    "reporting_interval_seconds": 5
  },
  "api": {
    "endpoint_url": "https://siem.corp.local/api/wifi-alerts",
    "custom_header_name": "X-API-KEY",
    "custom_header_value": "prod-key-abc123"
  },
  "hardware": {
    "buzzer_freq": 2500,
    "buzzer_duration_ms": 1000,
    "screen_brightness": 180,
    "fancy_intro": false
  },
  "debug": {
    "enabled": false
  }
}
```

### Battery-Saving Configuration

Optimized for extended portable operation:

```json
{
  "wifi": {
    "sta_ssid": "Network",
    "sta_password": "Password",
    "admin_user": "admin",
    "admin_pass": "admin"
  },
  "detection": {
    "protected_ssids": ["Network"],
    "reporting_interval_seconds": 60
  },
  "hardware": {
    "buzzer_duration_ms": 500,
    "screen_brightness": 50,
    "fancy_intro": false
  }
}
```

---

## Configuration Validation

The device validates configuration on load. Invalid configurations result in:

1. **Missing file:** Device enters configuration mode
2. **Invalid JSON:** Error displayed, configuration mode entered
3. **Missing required fields:** Defaults are applied where possible
4. **Invalid values:** May cause unexpected behavior

### Required Fields

The following fields are recommended for proper operation:

- `wifi.sta_ssid` — Required for NTP sync and API reporting
- `detection.protected_ssids` — Required for meaningful detection

### Validation Tips

- Use a JSON validator before saving
- Check serial output for configuration errors
- Test changes using the web interface first

---

## Next Steps

- [Web Interface Guide](web-interface.md) — Configure settings via browser
- [API Integration](api-integration.md) — Set up external reporting
