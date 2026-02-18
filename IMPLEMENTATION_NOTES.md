# DeauthDetector Implementation Notes

## Overview
This document describes the implementation of new configuration settings for the DeauthDetector class to provide more flexible and configurable deauthentication attack detection.

## Changes Implemented

### 1. Packet Threshold Configuration
**Requirement**: Set the limit to 250 and make it a constant and a setting in the config and website

**Implementation**:
- Added `DEFAULT_PACKET_THRESHOLD` constant in `Config.h` set to 250
- Added `packet_threshold` field to `DetectionConfig` struct
- Updated `ConfigManager` to load/save the setting from/to JSON config
- Updated `WebPortal` to display and save the setting in the web interface
- Modified `DeauthDetector::packetHandler()` to enforce the threshold and stop recording events once exceeded per BSSID

**Usage**: This prevents flooding the system with excessive deauth packets from a single source. Once a BSSID exceeds the threshold, its packets are ignored.

### 2. Detection Mode Configuration
**Requirement**: Add a config setting to detect only filtered SSIDs or to detect any deauth

**Implementation**:
- Added `detect_all_deauth` boolean field to `DetectionConfig` struct
- Updated `ConfigManager` to handle the checkbox setting
- Updated `WebPortal` to display a checkbox: "Detect All Deauth (not just protected SSIDs)"
- Added `shouldDetectDeauth()` method to `DeauthDetector` class
- Modified packet handler to support both filtered and unfiltered detection modes

**Usage**: 
- When `detect_all_deauth` is `false` (default): Only deauth packets targeting protected SSIDs are detected
- When `detect_all_deauth` is `true`: All deauth packets are detected regardless of target SSID

### 3. Channel Scan Time Configuration
**Requirement**: Make the time it takes to perform the scanning loop a setting

**Implementation**:
- Added `DEFAULT_CHANNEL_SCAN_TIME_MS` constant in `Config.h` set to 100ms
- Added `channel_scan_time_ms` field to `DetectionConfig` struct
- Updated `ConfigManager` to load/save the setting
- Updated `WebPortal` to display the setting with a minimum value of 50ms
- Modified `DeauthDetector::discoverChannels()` to use the configured scan time

**Usage**: Controls how long to scan each channel during the initial discovery phase. Lower values = faster scanning but may miss some networks. Higher values = more thorough but slower.

### 4. Channel Hop Interval Configuration
**Requirement**: Ensure the same channel hop setting is used for step 6 and for step 4 (consistent configuration)

**Implementation**:
- Added `DEFAULT_CHANNEL_HOP_INTERVAL_MS` constant in `Config.h` set to 500ms
- Added `channel_hop_interval_ms` field to `DetectionConfig` struct
- Updated `ConfigManager` to load/save the setting
- Updated `WebPortal` to display the setting with a minimum value of 100ms
- Added `updateChannelHop()` method to `DeauthDetector` class
- Added channel hopping state variables: `currentChannelIndex`, `lastChannelHopTime`
- Updated `main.cpp` to call `detector.updateChannelHop()` in the monitoring loop

**Usage**: Controls how frequently the detector switches between WiFi channels during monitoring. This ensures the detector can detect deauth attacks on any of the channels where protected SSIDs are located.

## Configuration File Format

Updated `config.txt.example` with new settings:

```json
{
  "detection": {
    "protected_ssids": ["Home_WiFi", "Office_Secure"],
    "silence_gap_seconds": 30,
    "led_hold_seconds": 300,
    "reporting_interval_seconds": 10,
    "packet_threshold": 250,
    "detect_all_deauth": false,
    "channel_scan_time_ms": 100,
    "channel_hop_interval_ms": 500
  }
}
```

## Files Modified

1. **include/Config.h**
   - Added constants for default values
   - Extended `DetectionConfig` struct with new fields

2. **src/ConfigManager.cpp**
   - Updated `setDefaults()` to initialize new settings
   - Updated `loadConfig()` to parse new settings from JSON
   - Updated `saveConfig()` to serialize new settings to JSON

3. **src/WebPortal.cpp**
   - Updated `handleSave()` to process new form fields
   - Updated `generateHTML()` to display new form fields in the detection tab

4. **include/DeauthDetector.h**
   - Modified `begin()` signature to accept `DetectionConfig`
   - Added new member variables for channel hopping and config storage
   - Added `updateChannelHop()` and `shouldDetectDeauth()` methods

5. **src/DeauthDetector.cpp**
   - Updated constructor to initialize new member variables
   - Modified `begin()` to store config and pass to `discoverChannels()`
   - Updated `discoverChannels()` to use configured scan time
   - Modified `startMonitoring()` to initialize channel hopping
   - Added `updateChannelHop()` implementation for periodic channel switching
   - Added `shouldDetectDeauth()` implementation for detection mode check
   - Updated `packetHandler()` to enforce packet threshold

6. **src/main.cpp**
   - Updated `detector.begin()` call to pass configuration
   - Added `detector.updateChannelHop()` call in monitoring loop

7. **config.txt.example**
   - Added new configuration fields with example values

## Testing Notes

Since this is an embedded hardware project requiring an M5 Cardputer device:

1. **Manual Testing Required**: The changes cannot be fully tested without the physical hardware
2. **Web Interface Testing**: After deployment, access the web configuration portal to verify:
   - New fields appear in the Detection tab
   - Settings can be saved and persist after restart
   - Validation (minimum values) works correctly

3. **Functional Testing**: With hardware:
   - Verify channel hopping occurs at the configured interval
   - Test packet threshold by monitoring a high-traffic network
   - Test detection modes (filtered vs all deauth)
   - Verify scan time affects discovery phase duration

## Code Quality

- All changes maintain backward compatibility with existing configurations (default values are provided)
- Constants are properly defined with clear naming
- No breaking changes to existing API
- Code follows existing project patterns and conventions
- All new settings are exposed in both the web interface and config file

## Potential Future Enhancements

1. Add validation in web interface for maximum reasonable values
2. Add real-time display of current channel in monitoring view
3. Implement BSSID-to-SSID mapping for better filtering in detect_all_deauth mode
4. Add statistics showing packets detected per channel
5. Consider adaptive channel hopping based on detection activity
