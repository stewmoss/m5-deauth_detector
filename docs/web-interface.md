# Web Interface Guide

The M5 Cardputer Deauth Detector includes a browser-based configuration portal for easy device management. This guide covers accessing the interface and using each configuration tab.

---

## Accessing the Web Interface

### Method 1: Access Point Mode (First Boot / Config Mode)

When the device is in configuration mode, it hosts its own WiFi network:

1. **Connect to the WiFi network:**
   - SSID: `M5-DeauthDetector`
   - Password: None (open network)

2. **Open a web browser and navigate to:**
   ```
   http://192.168.4.1
   ```

3. **Log in with credentials:**
   - Default Username: `admin`
   - Default Password: `1234`

### Method 2: From Monitor Mode

While the device is actively monitoring, you can enter configuration mode:

1. **Hold the G0 button** for 2 seconds
2. The device will switch to AP mode
3. Connect to `M5-DeauthDetector` and browse to `http://192.168.4.1`

### Authentication

The web interface is protected by HTTP Basic Authentication. Your browser will prompt for credentials before displaying the interface.

> ⚠️ **Security Recommendation:** Change the default credentials via the WiFi tab after first login.

---

## Interface Overview

The web interface uses a tabbed layout for organizing settings:

```
┌─────────────────────────────────────────────────────────────────┐
│            M5 Deauth Detector Configuration                     │
├──────────┬───────────┬─────────┬──────────┬─────────────────────┤
│  WiFi    │ Detection │   API   │ Hardware │       Debug         │
├──────────┴───────────┴─────────┴──────────┴─────────────────────┤
│                                                                 │
│   [Tab content displayed here]                                  │
│                                                                 │
│                                                                 │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                    [ Save Configuration ]                        │
└─────────────────────────────────────────────────────────────────┘
```

Each tab groups related settings. Changes are saved collectively when clicking the **Save Configuration** button.

---

## WiFi Tab

Configure network connectivity and portal security.

### Fields

| Field | Description | Example |
|-------|-------------|---------|
| **WiFi SSID** | Your home/office network name | `MyHomeNetwork` |
| **WiFi Password** | Network password | `MySecurePassword` |
| **Admin Username** | Web portal login username | `admin` |
| **Admin Password** | Web portal login password | `SecurePass123` |
| **NTP Server** | Time synchronization server | `pool.ntp.org` |
| **Timezone Offset** | Hours from UTC | `-5` for EST |
| **Daylight Savings** | Add 1 hour for DST | Checkbox |

### Usage Notes

- **WiFi SSID/Password:** Used for NTP time sync and API reporting. The device connects temporarily, then disconnects to resume monitoring.
- **Admin Credentials:** Protect access to the configuration portal. Change these from defaults.
- **NTP Server:** Common options include:
  - `pool.ntp.org` (global pool)
  - `time.google.com` (Google)
  - `time.windows.com` (Microsoft)
- **Timezone Offset:** Enter your UTC offset as a positive or negative integer.

### Example Configuration

```
WiFi SSID:        HomeNetwork
WiFi Password:    ••••••••••
Admin Username:   secadmin
Admin Password:   ••••••••
NTP Server:       time.google.com
Timezone Offset:  -8
[✓] Daylight Savings Time
```

---

## Detection Tab

Configure which networks to protect and alert timing.

### Fields

| Field | Description | Example |
|-------|-------------|---------|
| **Protected SSIDs** | Comma-separated list of networks to monitor | `Home_WiFi, Office_Network` |
| **Silence Gap** | Seconds of quiet before LED countdown starts | `30` |
| **LED Hold Time** | Seconds to keep LED red after silence | `300` |
| **Reporting Interval** | Seconds between API batch uploads | `10` |

### Usage Notes

- **Protected SSIDs:** Enter network names exactly as they broadcast. Case-sensitive.
  - Separate multiple SSIDs with commas
  - Spaces after commas are automatically trimmed
  - Example: `Home_2.4G, Home_5G, Guest_Network`

- **Silence Gap:** Controls how quickly the alert clears after attacks stop. Lower values = faster response. Higher values = fewer false "all clear" signals during ongoing attacks.

- **LED Hold Time:** How long the visual alert remains after attacks cease. Default 300 seconds (5 minutes) ensures you notice the alert even if away from the device.

- **Reporting Interval:** How often events are batched and sent to the API endpoint. Lower values = more real-time but more WiFi reconnections.

### Example Configuration

```
Protected SSIDs:     MyHome_WiFi, MyHome_5G, Office_Secure
Silence Gap:         60 seconds
LED Hold Time:       600 seconds
Reporting Interval:  15 seconds
```

---

## API Tab

Configure remote alerting to external systems.

### Fields

| Field | Description | Example |
|-------|-------------|---------|
| **API Endpoint URL** | Full URL for POST requests | `https://api.example.com/alerts` |
| **Custom Header Name** | Authentication header name | `X-API-KEY` |
| **Custom Header Value** | Authentication header value | `abc123secret` |

### Usage Notes

- **API Endpoint URL:** Must be a complete URL including protocol (`http://` or `https://`).
  - Leave empty to disable API reporting
  - Supports both HTTP and HTTPS
  - Example: `https://hooks.slack.com/services/...`

- **Custom Headers:** Used for authentication with your API.
  - Common patterns:
    - API Key: Name=`X-API-KEY`, Value=`your-key`
    - Bearer Token: Name=`Authorization`, Value=`Bearer your-token`
  - Leave both empty if no authentication required

### Example Configuration

```
API Endpoint URL:      https://security.mycompany.com/api/deauth
Custom Header Name:    Authorization
Custom Header Value:   Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6...
```

### Disabling API Reporting

Clear the **API Endpoint URL** field to disable remote reporting. Events will still be logged locally to the SD card.

---

## Hardware Tab

Configure physical device behavior.

### Fields

| Field | Description | Range | Default |
|-------|-------------|-------|---------|
| **Buzzer Frequency** | Alert tone frequency in Hz | 100-10000 | 2000 |
| **Buzzer Duration** | How long buzzer sounds in ms | 100-30000 | 2000 |
| **Screen Brightness** | Display brightness level | 0-255 | 128 |

### Usage Notes

- **Buzzer Frequency:** Higher values produce higher-pitched sounds.
  - 1000 Hz = Low tone
  - 2000 Hz = Medium tone (default)
  - 4000 Hz = High, attention-grabbing tone

- **Buzzer Duration:** How long the buzzer sounds when an attack is detected.
  - 500 ms = Brief beep
  - 2000 ms = Standard alert (2 seconds)
  - 5000 ms = Extended alert

- **Screen Brightness:** Adjust for your environment.
  - 0-50 = Very dim (battery saving)
  - 128 = Balanced
  - 200-255 = Maximum brightness

### Example Configuration

```
Buzzer Frequency:    2500 Hz
Buzzer Duration:     1500 ms
Screen Brightness:   180
```

---

## Debug Tab

Configure diagnostic logging for troubleshooting.

### Fields

| Field | Description |
|-------|-------------|
| **Enable Debug Logging** | Checkbox to enable/disable logging |

### Actions

| Button | Description |
|--------|-------------|
| **View Debug Log** | Opens debug log in new browser tab |
| **Clear Debug Log** | Deletes the current debug log file |

### Usage Notes

- **Debug Logging:** When enabled, detailed system messages are written to `/deauthdetector/logs/debug.log` on the SD card.

- **Log Contents Include:**
  - WiFi connection status
  - NTP sync results
  - Configuration loading messages
  - Detection events
  - API reporting status
  - Error messages

- **Log Management:**
  - Logs are automatically cleared on device reboot
  - Use "Clear Debug Log" to manually clear while device is running
  - "View Debug Log" displays the file contents in your browser

### When to Enable Debug Logging

Enable debug logging when:
- Troubleshooting connectivity issues
- Verifying detection is working correctly
- Diagnosing API reporting problems
- Providing information for support requests

> 💡 **Tip:** Disable debug logging during normal operation to save SD card write cycles.

---

## Saving Configuration

After making changes in any tab:

1. Click the **Save Configuration** button at the bottom
2. A confirmation message appears: "Configuration Saved!"
3. The device automatically restarts after 3 seconds
4. New settings take effect after restart

### Save Behavior

- All tab settings are saved together
- Invalid JSON is not possible via the web interface
- The device validates settings before restarting
- If save fails, an error message is displayed

---

## Session Timeout

The web portal has a **5-minute inactivity timeout**:

- Timer resets on any interaction (page load, save, etc.)
- After timeout, the device returns to Monitor Mode
- Reconnect to the AP and log in again to continue configuration

---

## Mobile Device Access

The web interface is fully responsive and works on mobile devices:

1. Connect your phone/tablet to `M5-DeauthDetector`
2. Open any mobile browser
3. Navigate to `http://192.168.4.1`
4. Log in normally

### Mobile Tips

- Disable mobile data to prevent routing issues
- Some phones may require "captive portal" dismissal
- Rotate to landscape for better tab visibility

---

## Troubleshooting

### Cannot Connect to AP

- Ensure device is in Configuration Mode (check display)
- Move closer to the device
- Disable VPN on your connecting device
- Try forgetting the network and reconnecting

### Login Fails

- Verify credentials are correct (case-sensitive)
- Clear browser cache and try again
- Check if someone else changed the password
- Reset device to defaults if needed

### Changes Not Saved

- Ensure SD card is properly inserted
- Check SD card isn't write-protected
- Verify sufficient space on SD card
- Check serial output for error messages

---

## Next Steps

- [Configuration Reference](configuration.md) — Detailed parameter documentation
- [Operation Guide](operation.md) — Using the device day-to-day
- [API Integration](api-integration.md) — External system integration
