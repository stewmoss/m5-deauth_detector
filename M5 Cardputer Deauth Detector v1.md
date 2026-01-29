This document serves as the final technical requirements specification for the **M5 Cardputer Deauth Detector**. It is designed to be handed directly to a developer for implementation using the ESP-IDF or Arduino (C++) framework.

# ---

**Technical Specification: M5 Cardputer Deauth Detector**

## **1\. Project Overview**

The goal is to build a handheld Wi-Fi security monitor using the **M5Stack Cardputer (StampS3)**. The device will monitor specific "Protected SSIDs" for deauthentication attacks, provide immediate hardware feedback (LED/Buzzer), maintain a local log on an SD card, and report incidents to a remote API.

## ---

**2\. Hardware Architecture & Pin Mapping**

* **Controller:** M5 StampS3 (ESP32-S3).  
* **Display:** 240x135 ST7789 TFT LCD.  
* **Input:** Full Keyboard (via I2C) \+ StampS3 "Go" Button (G0).  
* **Storage:** MicroSD Slot (SPI).  
* **Audio/Visual:** Built-in Buzzer and SK6812 RGB LED.

## ---

**3\. Operational Logic & State Machine**

### **3.1 Startup Sequence**

1. **Hardware Init:** Initialize SD card, Display, and Keyboard.  
2. **Config Load:** Read config.txt from SD. If missing or invalid, enter **Config Mode (AP/Captive Portal)** immediately.  
3. **Time Sync:** Connect to Wi-Fi, sync time via **NTP**, then disconnect.  
4. **Discovery Scan:** Briefly scan all 14 channels to identify which channels the "Protected SSIDs" are broadcasting on.  
5. **Enter Monitor Mode.**

### **3.2 Monitor Mode (The "Sniffer")**

* **Channel Hopping:** The device only hops between channels identified during the Discovery Scan.  
* **Detection:** Listen for Management Frames where Subtype \== 0x0C (Deauthentication).  
* **Alerting:** \* **Buzzer:** On detection, sound for $X$ seconds (defined in config).  
  * **LED:** Turn **Solid Red**. Start a 5-minute countdown timer only after a "Silence Gap" (no packets detected for $Y$ seconds). Any new packet resets the 5-minute timer.  
* **Batching:** All detections are queued in RAM. Every 10 seconds (configurable), the device pauses sniffing, connects to Wi-Fi, sends the **Bulk JSON POST**, and resumes sniffing.

### **3.3 Config Mode (Web Server)**

* **Access:** Triggered by Enter key, "Go" button, or boot-up.  
* **Interface:** A tabbed web UI (Password Protected) hosted at 192.168.4.1 (AP Mode) or the local IP (STA Mode).  
* **Duration:** Remains active for 5 minutes of inactivity before returning to Monitor Mode.

## ---

**4\. Configuration Specification (config.txt)**

The configuration must be stored in a plain-text format (JSON or INI) on the root of the SD card.

JSON

{  
  "wifi": {  
    "sta\_ssid": "Your\_Home\_Network",  
    "sta\_password": "Your\_Password",  
    "admin\_user": "admin",  
    "admin\_pass": "1234"  
  },  
  "ntp": {  
    "server": "pool.ntp.org",  
    "timezone\_offset": 0,  
    "daylight\_savings": true  
  },  
  "detection": {  
    "protected\_ssids": \["Home\_WiFi", "Office\_Secure"\],  
    "silence\_gap\_seconds": 30,  
    "led\_hold\_seconds": 300,  
    "reporting\_interval\_seconds": 10  
  },  
  "api": {  
    "endpoint\_url": "https://your-api.com/v1/alerts",  
    "custom\_header\_name": "X-API-KEY",  
    "custom\_header\_value": "secret-token-123"  
  },  
  "hardware": {  
    "buzzer\_freq": 2000,  
    "buzzer\_duration\_ms": 2000,  
    "screen\_brightness": 128  
  }  
}

## ---

**5\. API Payload Specification**

Data must be sent as a **JSON Array** of objects via a **POST** request.

**Endpoint:** Configurable (supports HTTP/HTTPS).

**Method:** POST

**Example Payload:**

JSON

\[  
  {  
    "timestamp": "2026-01-13T10:15:00Z",  
    "target\_ssid": "Home\_WiFi",  
    "target\_bssid": "AA:BB:CC:DD:EE:FF",  
    "attacker\_mac": "11:22:33:44:55:66",  
    "channel": 6,  
    "rssi": \-45,  
    "packet\_count": 24  
  },  
  {  
    "timestamp": "2026-01-13T10:15:08Z",  
    "target\_ssid": "Home\_WiFi",  
    "target\_bssid": "AA:BB:CC:DD:EE:FF",  
    "attacker\_mac": "Unknown",  
    "channel": 6,  
    "rssi": \-50,  
    "packet\_count": 12  
  }  
\]

## ---

**6\. UI & Data Visualization**

The user can cycle through three views using the **Go** button or **Enter** key:

| View | Description |
| :---- | :---- |
| **Dashboard** | A list of Protected SSIDs with a cumulative counter of attacks for the current session. |
| **Live Log** | A scrolling list showing the 5 most recent events (Time |
| **Detailed Rotation** | Shows one SSID per page with expanded data: Total packets, Last Attacker MAC, and Last Attack Timestamp. |

## ---

**7\. Logging Requirements**

* **Filename:** logs/session\_YYYYMMDD\_HHMMSS.csv  
* **Format:** CSV or JSON lines.  
* **Content:** Every detection entry must be appended to this file as it occurs to ensure data isn't lost if the API call fails.

---

This concludes the specification. Would you like me to refine the visual design of the Web Configuration UI or provide a code snippet for the Wi-Fi toggling logic?