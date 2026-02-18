# Troubleshooting Guide

This guide covers common issues, their causes, and solutions for the M5 Cardputer Deauth Detector.

---

## Quick Diagnostics

Before diving into specific issues, gather diagnostic information:

### Enable Debug Logging

1. Access the web interface
2. Navigate to the **Debug** tab
3. Check **Enable Debug Logging**
4. Save and restart
5. Reproduce the issue
6. View debug log via web interface or SD card

### Serial Monitor Output

Connect via USB and monitor serial output:

```bash
pio device monitor
```

This shows real-time system messages useful for diagnosing startup issues.

---

## Startup Issues

### SD CARD ERROR

**Symptom:** Red screen with "SD CARD ERROR!" message

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Card not inserted | Insert SD card fully into slot |
| Card not FAT32 formatted | Reformat as FAT32 |
| Card corrupted | Try a different SD card |
| Card contacts dirty | Clean contacts with isopropyl alcohol |
| Incompatible card | Use a different brand/capacity card |

**Formatting Instructions:**

*Windows:*
1. Right-click drive → Format
2. Select FAT32
3. Click Start

*macOS:*
1. Open Disk Utility
2. Select SD card → Erase
3. Format: MS-DOS (FAT)

*Linux:*
```bash
sudo mkfs.vfat -F 32 /dev/sdX1
```

### Configuration Not Loading

**Symptom:** Device enters config mode despite having config.txt

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| File named incorrectly | Rename to exactly `config.txt` (lowercase) |
| File in wrong location | Move to SD card root, not in a folder |
| Invalid JSON syntax | Validate JSON using online validator |
| Encoding issues | Save as UTF-8 without BOM |
| Missing required fields | Check against example configuration |

**JSON Validation:**

1. Copy your config.txt content
2. Paste into [jsonlint.com](https://jsonlint.com/)
3. Click "Validate JSON"
4. Fix any reported errors

**Common JSON Errors:**

```json
// ❌ Wrong: Trailing comma
"protected_ssids": ["Network1", "Network2",]

// ✅ Correct: No trailing comma
"protected_ssids": ["Network1", "Network2"]
```

```json
// ❌ Wrong: Single quotes
'sta_ssid': 'MyNetwork'

// ✅ Correct: Double quotes
"sta_ssid": "MyNetwork"
```

### Device Stuck on Startup

**Symptom:** Device hangs during initialization

**Causes and Solutions:**

1. **Hardware issue:** Try power cycling (unplug, wait 5 seconds, replug)
2. **SD card problem:** Try booting without SD card (enters config mode)
3. **Corrupted firmware:** Re-flash using PlatformIO
4. **Memory issue:** Remove old session log files from SD card

---

## WiFi Issues

### Cannot Connect to WiFi

**Symptom:** "Connecting to WiFi..." never completes

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Wrong SSID | Verify exact spelling (case-sensitive) |
| Wrong password | Check password accuracy |
| 5 GHz network | ESP32 only supports 2.4 GHz |
| Network out of range | Move closer to access point |
| Too many devices | Check AP client limit |
| MAC filtering | Add device MAC to allowed list |

**Verify Network Compatibility:**

- Network must be 2.4 GHz (802.11 b/g/n)
- WPA/WPA2 Personal supported
- WPA2 Enterprise (802.1X) not supported
- Hidden SSIDs may require special handling

### WiFi AP Not Visible

**Symptom:** Cannot see `M5-DeauthDetector` network

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Device in Monitor Mode | Hold G0 for 2 seconds to enter config mode |
| Startup not complete | Wait for initialization to finish |
| Interference | Move away from other 2.4 GHz devices |
| Phone/laptop issue | Try a different device |

**Forcing AP Mode:**

1. Power off the device
2. Remove SD card (or delete config.txt)
3. Power on—device will enter config mode
4. Reconfigure via web interface

### WiFi Disconnects Frequently

**Symptom:** NTP sync fails, API reports don't send

**Causes and Solutions:**

- Check signal strength (move closer to router)
- Verify router isn't overloaded
- Try a static channel on your router (not "Auto")
- Check for WiFi interference sources

---

## Detection Issues

### No Deauth Detection

**Symptom:** Device monitoring but never detects attacks

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| SSIDs spelled incorrectly | Verify exact SSID names in config |
| Networks not broadcasting | Ensure networks are visible |
| Wrong band | Protected networks must be 2.4 GHz |
| Networks not found during scan | Check networks are active at startup |
| No actual attacks | Device is working correctly |

**Verifying Protected Networks:**

1. Check serial output during startup for "Found [SSID] on channel X"
2. If not found, the SSID may be:
   - Spelled differently
   - On 5 GHz band
   - Currently offline
   - Out of range during channel discovery

### False Positives

**Symptom:** Alerts without actual attacks

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Legitimate deauths | Some devices send deauths during roaming |
| Nearby testing | Others may be doing security assessments |
| Interference | Strong RF interference can cause issues |

**Reducing False Positives:**

1. Increase silence gap to filter brief events
2. Review logs to identify patterns
3. Consider physical shielding if needed

### Detection Delayed

**Symptom:** Attacks detected but not immediately

**Causes and Solutions:**

- This is normal—device hops between channels
- Only monitored channels are scanned (more SSIDs = slower hop)
- Consider reducing protected SSIDs to critical networks only

---

## Display Issues

### Screen Blank or Garbled

**Symptom:** Display not showing content correctly

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Brightness set to 0 | Reconfigure via web interface |
| Hardware issue | Check display ribbon cable |
| Firmware issue | Re-flash firmware |

### Display Too Dim/Bright

**Symptom:** Screen hard to read

**Solution:**

1. Access web interface
2. Go to Hardware tab
3. Adjust Screen Brightness (0-255)
4. Save configuration

---

## Alert Issues

### Buzzer Not Working

**Symptom:** No sound when attacks detected

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Volume/frequency too low | Increase buzzer_freq in config |
| Duration too short | Increase buzzer_duration_ms |
| Hardware issue | Test with known working firmware |

**Testing Buzzer:**

The buzzer should briefly sound during the animated intro (if enabled).

### LED Not Lighting

**Symptom:** LED doesn't change color during alerts

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| LED hardware issue | Check for physical damage |
| FastLED library issue | Re-flash firmware |
| Wrong GPIO configured | Verify firmware pin assignments |

---

## Web Interface Issues

### Cannot Access Web Portal

**Symptom:** Browser cannot reach 192.168.4.1

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Not connected to AP | Connect to M5-DeauthDetector first |
| Wrong IP address | Confirm 192.168.4.1 is correct |
| Mobile data interfering | Disable mobile data temporarily |
| VPN active | Disable VPN |
| Browser cache | Try incognito/private window |

**Troubleshooting Steps:**

1. Verify connected to `M5-DeauthDetector` network
2. Check assigned IP (should be 192.168.4.x)
3. Try: `http://192.168.4.1` (include http://)
4. Disable any VPN or proxy
5. Try a different browser

### Login Fails

**Symptom:** Credentials rejected

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Wrong credentials | Try default: admin / 1234 |
| Credentials changed | Check config.txt on SD card |
| Browser caching old creds | Clear browser cache |

**Resetting Credentials:**

1. Remove SD card
2. Open config.txt on computer
3. Change `admin_user` and `admin_pass`
4. Save and reinsert SD card
5. Restart device

### Configuration Won't Save

**Symptom:** Save button doesn't work or error appears

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| SD card full | Delete old log files |
| SD card read-only | Check physical write-protect switch |
| SD card disconnected | Ensure card is seated properly |
| Session timeout | Refresh page and re-login |

---

## API Reporting Issues

### Events Not Reaching API

**Symptom:** No data at API endpoint

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| URL incorrect | Verify complete URL with protocol |
| Authentication failing | Check header name/value |
| Network issues | Verify WiFi connectivity |
| Endpoint unreachable | Test URL with curl/Postman |
| HTTPS certificate issue | Try HTTP for testing |

**Testing API Endpoint:**

```bash
# Test with curl
curl -X POST https://your-api.com/endpoint \
  -H "Content-Type: application/json" \
  -H "X-API-KEY: your-key" \
  -d '[{"test": "data"}]'
```

### API Timeout Errors

**Symptom:** Debug log shows API timeouts

**Causes and Solutions:**

- Check API endpoint response time
- Verify network stability
- Increase reporting interval
- Check API rate limits

---

## Log File Issues

### Logs Not Being Written

**Symptom:** No session log files on SD card

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| SD card full | Delete old files |
| Directory missing | Let device recreate on boot |
| SD card issue | Try different card |
| No events to log | Device working—no attacks detected |

### Debug Log Empty

**Symptom:** Debug log file exists but is empty

**Causes and Solutions:**

- Debug logging may be disabled—enable in config
- Device may have just booted (log cleared on boot)
- Check config has `"debug": { "enabled": true }`

---

## Performance Issues

### Device Slow or Unresponsive

**Symptom:** Laggy display, missed detections

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Too many protected SSIDs | Reduce to essential networks |
| SD card slow | Use faster SD card (Class 10) |
| Memory fragmentation | Restart device |
| API endpoint slow | Increase reporting interval |

### High Battery Drain

**Symptom:** Battery depletes quickly

**Solutions:**

1. Reduce screen brightness
2. Increase reporting interval
3. Disable fancy intro
4. Disable debug logging

---

## Factory Reset Procedure

To completely reset the device:

1. **Power off** the device
2. **Remove the SD card**
3. **Delete** `config.txt` from the SD card
4. **Optionally delete** the `/deauthdetector` folder
5. **Reinsert** the SD card
6. **Power on**—device enters configuration mode
7. **Reconfigure** via web interface

---

## Getting Help

If issues persist after trying these solutions:

### Information to Gather

1. **Debug log** content
2. **Serial monitor** output during issue
3. **Configuration** (redact passwords)
4. **Firmware version** and build date
5. **Steps to reproduce** the issue

### Debug Log Retrieval

1. Enable debug logging
2. Reproduce the issue
3. Access web interface → Debug tab → View Debug Log
4. Copy the log content

### Serial Output Capture

```bash
# Start monitoring
pio device monitor > debug_output.txt

# Reproduce issue, then Ctrl+C to stop
```

---

## Common Error Messages

| Message | Meaning | Solution |
|---------|---------|----------|
| `SD CARD ERROR!` | SD card not readable | Check card format and insertion |
| `Config not found` | No valid config.txt | Create or fix config file |
| `JSON parse error` | Invalid config syntax | Validate JSON format |
| `WiFi connect failed` | Cannot reach network | Check SSID/password, signal |
| `NTP sync failed` | Time server unreachable | Check internet connectivity |
| `API POST failed` | Cannot reach endpoint | Verify URL and auth |

---

## Related Documentation

- [Getting Started](getting-started.md) — Initial setup
- [Configuration Reference](configuration.md) — All settings explained
- [Web Interface Guide](web-interface.md) — Portal usage
- [Operation Guide](operation.md) — Daily use
