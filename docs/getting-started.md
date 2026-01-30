# Getting Started

This guide walks you through setting up the M5 Cardputer Deauth Detector from initial hardware preparation to your first monitoring session.

---

## Prerequisites

### Hardware Requirements

| Item | Notes |
|------|-------|
| **M5Stack Cardputer** | ESP32-S3 based model (StampS3) |
| **MicroSD Card** | Minimum 1GB, FAT32 formatted |
| **USB-C Cable** | For programming and power |
| **Computer** | Windows, macOS, or Linux with USB port |

### Software Requirements

| Software | Purpose |
|----------|---------|
| **VS Code** | Development environment |
| **PlatformIO Extension** | Build system and upload tool |
| **USB Drivers** | CH340 or CP210x (depending on your system) |

---

## Installation

### Step 1: Install Development Environment

1. Download and install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code and navigate to the Extensions panel (Ctrl+Shift+X)
3. Search for "PlatformIO IDE" and install it
4. Restart VS Code after installation completes

### Step 2: Clone or Download the Project

```bash
# Clone via Git
git clone https://github.com/your-repo/deauth_detector.git

# Or download and extract the ZIP file
```

### Step 3: Open in PlatformIO

1. Open VS Code
2. Click **File → Open Folder**
3. Navigate to the `deauth_detector` project folder
4. PlatformIO will automatically detect the project and download dependencies

### Step 4: Connect the Cardputer

1. Connect your M5 Cardputer to your computer via USB-C
2. The device should appear as a serial port
3. Verify in PlatformIO: Click the **PlatformIO icon** → **Devices** to see connected ports

### Step 5: Build and Upload

**Option A: Using the PlatformIO GUI**

1. Click the **PlatformIO icon** in the sidebar
2. Under **Project Tasks**, click **Upload**
3. Wait for compilation and upload to complete

**Option B: Using the Terminal**

```bash
# Navigate to project directory
cd deauth_detector

# Build and upload
pio run --target upload
```

### Step 6: Monitor Serial Output (Optional)

To view debug messages during startup:

```bash
pio device monitor
```

Or use the **Serial Monitor** button in PlatformIO.

---

## Preparing the SD Card

### Formatting Requirements

The SD card must be formatted as **FAT32**. This is required for the ESP32's SD library compatibility.

**Windows:**
1. Insert the SD card
2. Right-click the drive → **Format**
3. Select **FAT32** as the file system
4. Click **Start**

**macOS:**
1. Open **Disk Utility**
2. Select the SD card
3. Click **Erase**
4. Choose **MS-DOS (FAT)** format

**Linux:**
```bash
# Replace /dev/sdX with your SD card device
sudo mkfs.vfat -F 32 /dev/sdX1
```

### Optional: Pre-configure the Device

You can optionally pre-configure the device by placing a `config.txt` file on the SD card root:

1. Copy `config.txt.example` from the project folder to the SD card
2. Rename it to `config.txt`
3. Edit the file with your settings (see [Configuration Reference](configuration.md))

If no configuration file is present, the device will enter configuration mode on first boot.

---

## First Boot

### Initial Startup Sequence

1. Insert the prepared SD card into the Cardputer
2. Power on the device (USB or battery)
3. The startup screen displays "Deauth Detector - Initializing..."

### Scenario A: No Configuration Found

If the device cannot find a valid `config.txt`:

1. The device automatically enters **Configuration Mode**
2. A WiFi access point is created:
   - **SSID:** `M5-DeauthDetector`
   - **Password:** None (open network)
3. The display shows the AP status and IP address

**Next steps:**
1. Connect to `M5-DeauthDetector` from your phone or computer
2. Open a browser and navigate to `http://192.168.4.1`
3. Log in with default credentials:
   - **Username:** `admin`
   - **Password:** `1234`
4. Configure your settings (see [Web Interface Guide](web-interface.md))
5. Click **Save Configuration**
6. The device will restart and begin monitoring

### Scenario B: Valid Configuration Found

If a valid `config.txt` exists:

1. The animated intro plays (if enabled)
2. The device connects to your configured WiFi network
3. Time is synchronized via NTP
4. Protected SSID channels are discovered
5. Monitoring begins automatically

---

## Verifying Operation

### Display Indicators

After successful startup, you should see the **Dashboard View**:

```
┌────────────────────────────────────────┐
│ Deauth Detector         [Monitor Mode] │
├────────────────────────────────────────┤
│                                        │
│   Protected Networks                   │
│   ─────────────────                    │
│   Home_WiFi          CH:6    Attacks:0 │
│   Office_Secure      CH:11   Attacks:0 │
│                                        │
│   Monitoring...                        │
│                                        │
└────────────────────────────────────────┘
```

### LED Status Indicators

| Color | Status |
|-------|--------|
| **Off** | Normal monitoring, no recent attacks |
| **Yellow** | Connecting to WiFi |
| **Cyan** | Syncing time via NTP |
| **Purple/Magenta** | Scanning for protected SSIDs |
| **Red** | Attack detected (active alert) |

### Testing Alert Functionality

To verify the device is working correctly, you can:

1. **Check the buzzer:** A brief tone plays during startup
2. **Monitor serial output:** Debug messages confirm detection engine status
3. **Simulate an attack:** Use a controlled test environment with appropriate tools

> ⚠️ **Warning:** Only test deauthentication attacks on networks you own or have explicit permission to test. Unauthorized attacks are illegal in most jurisdictions.

---

## Next Steps

Your Deauth Detector is now operational. Continue with these guides:

- [Configuration Reference](configuration.md) — Customize all device settings
- [Web Interface Guide](web-interface.md) — Learn to use the configuration portal
- [Operation Guide](operation.md) — Understand display views and alerts

---

## Quick Troubleshooting

| Issue | Solution |
|-------|----------|
| **SD CARD ERROR** on screen | Ensure card is FAT32 formatted and properly inserted |
| **Device doesn't power on** | Verify USB connection or charge battery |
| **Cannot upload firmware** | Check USB drivers, try different cable/port |
| **WiFi AP not visible** | Wait 30 seconds, check if device is in monitor mode |

For more detailed troubleshooting, see [Troubleshooting Guide](troubleshooting.md).
