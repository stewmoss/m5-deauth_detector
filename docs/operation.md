# Operation Guide

This guide covers daily use of the M5 Cardputer Deauth Detector, including display views, alert interpretation, and operational procedures.

---

## Startup Sequence

When the device powers on with a valid configuration:

### 1. Initialization Phase

```
┌────────────────────────────┐
│                            │
│       Deauth               │
│       Detector             │
│                            │
│     Initializing...        │
│ By Stewart Moss (c) 2026   │
└────────────────────────────┘
```

If `fancy_intro` is enabled, an animated radar sweep plays with scanning sounds.

### 2. WiFi Connection

The device connects to your configured WiFi network:

```
Connecting to WiFi... MyNetwork
```

**LED Status:** Yellow (connecting)

### 3. Time Synchronization

```
Syncing time...
```

**LED Status:** Cyan (syncing)

Time is synchronized via NTP for accurate event timestamps.

### 4. Channel Discovery

The device scans all 14 Wi-Fi channels (2.4 GHz) to locate your protected networks:

```
Scanning for protected SSIDs...
```

**LED Status:** Purple/Magenta (scanning)

Only channels where protected SSIDs are found will be monitored.

### 5. Monitor Mode

**LED Status:** Off (ready/monitoring)

The device enters promiscuous mode and begins monitoring for deauthentication attacks.

---

## Display Views

Press **ENTER** to cycle through three display views:

### View 1: Dashboard

The primary monitoring display showing all protected networks at a glance.

```
┌────────────────────────────────────────┐
│ Deauth Detector         [Monitor Mode] │
├────────────────────────────────────────┤
│                                        │
│   Protected Networks                   │
│   ─────────────────                    │
│   Home_WiFi          CH:6    Attacks:0 │
│   Office_Secure      CH:11   Attacks:2 │
│   Guest_Network      CH:1    Attacks:0 │
│                                        │
│   Monitoring... [12:34:56]             │
│                                        │
└────────────────────────────────────────┘
```

**Information displayed:**
- Network name (SSID)
- Channel number where network was found
- Cumulative attack count for current session
- Current time

### View 2: Live Log

Displays the 5 most recent detection events in reverse chronological order.

```
┌────────────────────────────────────────┐
│ Live Event Log                         │
├────────────────────────────────────────┤
│                                        │
│   14:23:15  Office_Secure  RSSI:-42    │
│   14:22:58  Office_Secure  RSSI:-38    │
│   14:20:01  Home_WiFi      RSSI:-55    │
│   13:45:22  Home_WiFi      RSSI:-61    │
│   13:45:20  Home_WiFi      RSSI:-58    │
│                                        │
│   Events: 5 total                      │
│                                        │
└────────────────────────────────────────┘
```

**Information displayed:**
- Timestamp of detection
- Target network name
- Signal strength (RSSI) of the deauth frame

### View 3: Detailed View

Shows detailed information for each protected SSID, one per page. Press ENTER to cycle through SSIDs.

```
┌────────────────────────────────────────┐
│ Network Details: Home_WiFi             │
├────────────────────────────────────────┤
│                                        │
│   BSSID:      AA:BB:CC:DD:EE:FF        │
│   Channel:    6                        │
│   Attacks:    3                        │
│                                        │
│   Last Attack:                         │
│   Time:       14:20:01                 │
│   Attacker:   11:22:33:44:55:66        │
│   Packets:    24                       │
│   RSSI:       -55 dBm                  │
│                                        │
│   [1/3] Press ENTER for next           │
└────────────────────────────────────────┘
```

**Information displayed:**
- Network BSSID (access point MAC)
- Channel number
- Total attack count
- Last attack timestamp
- Last attacker MAC address
- Packet count from last attack
- Signal strength of last attack

---

## LED Status Indicators

The SK6812 RGB LED provides at-a-glance status:

| Color | State | Meaning |
|-------|-------|---------|
| **Off** | Solid | Normal operation, no recent attacks |
| **Yellow** | Solid | Connecting to WiFi |
| **Cyan** | Solid | Syncing time via NTP |
| **Purple** | Solid | Scanning for protected networks |
| **Red** | Solid | Attack detected / active alert |

### Alert LED Behavior

When an attack is detected:

1. **Immediate:** LED turns solid red
2. **Continuous:** LED remains red while packets are being received
3. **Silence Gap:** After configured seconds of quiet, countdown begins
4. **LED Hold:** LED stays red for configured hold duration
5. **Clear:** LED turns off when hold period expires

Any new attack during the hold period resets the countdown.

---

## Buzzer Alerts

When deauthentication packets are detected, the buzzer sounds:

- **Frequency:** Configured in Hardware settings (default: 2000 Hz)
- **Duration:** Configured in Hardware settings (default: 2000 ms)
- **Trigger:** First packet of a new attack

The buzzer provides immediate audible notification, useful when the device is not in direct line of sight.

---

## Entering Configuration Mode

To access the web interface while the device is monitoring:

### Method: G0 Button Hold

1. Locate the **G0 button** on the M5 Cardputer (hardware button, not keyboard)
2. **Press and hold** for approximately 2 seconds
3. Release when the display indicates "Entering Config Mode"
4. Device creates WiFi AP: `M5-DeauthDetector`
5. Connect and browse to `http://192.168.4.1`

### Automatic Return to Monitoring

The web portal has a 5-minute inactivity timeout. After timeout:

1. Web server stops
2. WiFi AP is disabled
3. Device returns to Monitor Mode automatically

---

## Understanding Detections

### What Gets Detected

The device monitors for IEEE 802.11 deauthentication frames (management frame subtype 0x0C) that:

- Target BSSIDs matching your protected networks
- Are broadcast or targeted deauthentications
- Occur on monitored channels

### What Gets Logged

Each detection event records:

| Field | Description |
|-------|-------------|
| `timestamp` | ISO 8601 formatted time |
| `target_ssid` | Network name being attacked |
| `target_bssid` | MAC address of the access point |
| `attacker_mac` | Source MAC of the deauth frame |
| `channel` | Wi-Fi channel of the attack |
| `rssi` | Signal strength in dBm |
| `packet_count` | Number of deauth packets in this event |

### Interpreting Attack Strength

| RSSI Value | Interpretation |
|------------|----------------|
| -30 to -40 | Very strong, attacker is very close |
| -40 to -55 | Strong, attacker is nearby (same room/building) |
| -55 to -70 | Medium, attacker could be outside building |
| -70 to -85 | Weak, attacker at greater distance |
| Below -85 | Very weak, may be edge of range |

---

## Log Files

### Session Logs

Location: `/deauthdetector/logs/deauthdetect_session_YYYYMMDD_HHMMSS.csv`

A new session log is created each time the device boots. Format:

```csv
timestamp,target_ssid,target_bssid,attacker_mac,channel,rssi,packet_count
2026-01-30T14:20:01Z,Home_WiFi,AA:BB:CC:DD:EE:FF,11:22:33:44:55:66,6,-55,24
2026-01-30T14:22:58Z,Office_Secure,DD:EE:FF:AA:BB:CC,77:88:99:AA:BB:CC,11,-38,12
```

### Debug Logs

Location: `/deauthdetector/logs/debug.log`

When debug logging is enabled, system messages are captured:

```
[12:00:01] Config loaded successfully
[12:00:02] Connecting to WiFi: MyNetwork
[12:00:05] WiFi connected, IP: 192.168.1.100
[12:00:06] NTP sync successful
[12:00:10] Found Home_WiFi on channel 6
[12:00:11] Found Office_Secure on channel 11
[12:00:12] Starting packet monitoring...
[14:20:01] ALERT: Deauth detected on Home_WiFi
```

---

## API Reporting

Events are batched and sent to your configured API endpoint:

### Reporting Cycle

1. Events are queued in RAM as they occur
2. At configured intervals (default: 10 seconds):
   - Device pauses monitoring briefly
   - Connects to WiFi
   - Sends queued events as JSON POST
   - Disconnects and resumes monitoring

### If API Reporting Fails

- Events remain in the queue for retry
- Local SD card logging is always performed
- Device continues monitoring regardless of API status

For API payload format, see [API Integration](api-integration.md).

---

## Power Management

### USB Power

When connected to USB:
- Device operates continuously
- No power concerns
- Recommended for fixed installations

### Battery Operation

When running on battery:
- Monitor screen brightness (lower = longer life)
- Consider longer reporting intervals
- Disable fancy intro animation
- Battery life varies by usage pattern

### Power Consumption Tips

| Setting | Lower Power | Higher Power |
|---------|-------------|--------------|
| Screen Brightness | 50-100 | 200-255 |
| Reporting Interval | 60+ seconds | 5-10 seconds |
| Debug Logging | Disabled | Enabled |
| Fancy Intro | Disabled | Enabled |

---

## Operational Best Practices

### Placement

- Position with clear line-of-sight to monitored networks
- Central location for balanced channel coverage
- Avoid metal enclosures that block RF signals
- USB power for permanent installations

### Monitoring Strategy

1. **Define your protected networks** — Include all critical SSIDs
2. **Set appropriate thresholds** — Balance sensitivity vs. false alarms
3. **Configure API reporting** — Integrate with existing security infrastructure
4. **Enable debug logging initially** — Verify proper operation
5. **Disable debug logging** — After confirming everything works

### Response Procedure

When an alert occurs:

1. **Acknowledge** — Note the time and affected network
2. **Investigate** — Check for nearby devices, unusual activity
3. **Document** — Review logs on SD card or via web interface
4. **Respond** — Take appropriate action based on your security policy
5. **Report** — If API is configured, events are automatically forwarded

---

## Keyboard Shortcuts Reference

| Key | Action |
|-----|--------|
| **ENTER** | Cycle through display views |
| **G0 Button (hold)** | Enter configuration mode |

---

## Next Steps

- [API Integration](api-integration.md) — Set up external reporting
- [Troubleshooting](troubleshooting.md) — Solve common issues
- [Configuration Reference](configuration.md) — Fine-tune settings
