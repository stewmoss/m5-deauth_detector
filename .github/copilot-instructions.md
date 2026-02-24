# GitHub Copilot Instructions — M5 Cardputer Projects

These instructions define the canonical design patterns for all M5 Cardputer projects.
Always follow these conventions unless a project's requirements explicitly require deviation.

---

## 1. Project & File Structure

All projects follow this structure:

```
<project-root>/
├── .github/
│   └── copilot-instructions.md
├── docs/
│   ├── index.md
│   ├── getting-started.md
│   ├── configuration.md
│   ├── web-interface.md
│   ├── operation.md
│   └── troubleshooting.md
├── include/
│   ├── Config.h          # All structs and constants
│   ├── ConfigManager.h
│   ├── Display.h
│   ├── AlertManager.h
│   ├── Logger.h
│   ├── WebPortal.h
│   ├── WiFiManager.h
│   └── <Feature>.h       # One header per module
├── src/
│   ├── main.cpp
│   ├── ConfigManager.cpp
│   ├── Display.cpp
│   ├── AlertManager.cpp
│   ├── Logger.cpp
│   ├── WebPortal.cpp
│   ├── WiFiManager.cpp
│   └── <Feature>.cpp     # One source per module
├── output/               # Created automatically by rename_firmware.py (do not commit binaries)
├── config.txt.example    # Documented example configuration
├── platformio.ini
├── rename_firmware.py
└── README.md
```

**Rules:**
- Every module has exactly one `.h` in `include/` and one `.cpp` in `src/`.
- `Config.h` is the single source of truth for all structs, constants, and defaults.
- `main.cpp` owns the application state machine and the `setup()`/`loop()` entry points.
- Never add business logic to `main.cpp` beyond state transitions and initialization sequencing.

---

## 2. SD Card Directory & Startup Initialization

Every project has a dedicated application directory on the SD card, created during `setup()`.
The directory name matches the project name in lowercase, no spaces (e.g., `/deauthdetector`).

### Pattern

```cpp
// In main.cpp setup()

// Define SD SPI pins — M5 Cardputer hardware constants
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    // Show error on display and halt
    M5Cardputer.Display.fillScreen(RED);
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.println("SD CARD ERROR!");
    while (1) delay(1000);
}

// Create application directory
if (!SD.exists("/<appname>")) {
    if (!SD.mkdir("/<appname>")) {
        M5Cardputer.Display.println("Cant create /<appname>");
        while (1) delay(1000);
    }
}
```

**Rules:**
- SD initialization always happens before config loading.
- SD failure always halts with a red screen and a clear error message.
- All application files (config, logs, sessions) live under `/<appname>/`.
- The default config path is `/<appname>/<appname>config.txt`.

---

## 3. Configuration Pattern

### Config.h — Struct Definitions

All configuration is defined as nested structs in `Config.h`.
Group settings by concern: `WiFiConfig`, `NTPConfig`, `HardwareConfig`, `DebugConfig`, plus any app-specific groups.
Collect all groups into a top-level `AppConfig` struct.

```cpp
// Config.h

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

// Default constants — always define defaults here, not in ConfigManager
#define DEFAULT_SCREEN_BRIGHTNESS 128

struct WiFiConfig {
    String sta_ssid;
    String sta_password;
    String admin_user;
    String admin_pass;
};

struct NTPConfig {
    String server;
    int timezone_offset;
    bool daylight_savings;
};

struct HardwareConfig {
    int buzzer_freq;
    int buzzer_duration_ms;
    int screen_brightness;
    bool fancy_intro;
};

struct DebugConfig {
    bool enabled;
};

struct AppConfig {
    WiFiConfig wifi;
    NTPConfig ntp;
    HardwareConfig hardware;
    DebugConfig debug;
    // Add feature-specific config groups here
};
```

### ConfigManager — Loading & Saving

`ConfigManager` reads and writes `AppConfig` to/from a JSON file on the SD card using ArduinoJson.

```cpp
class ConfigManager {
public:
    ConfigManager();
    bool loadConfig(const char* filename = "/<appname>/<appname>config.txt");
    bool saveConfig(const char* filename = "/<appname>/<appname>config.txt");
    AppConfig& getConfig() { return config; }
    bool isValid() { return configValid; }

private:
    AppConfig config;
    bool configValid;
    void setDefaults();
};
```

**Rules:**
- `setDefaults()` is always called before loading so missing keys fall back gracefully.
- Config is JSON with one top-level object per struct group (e.g., `"wifi"`, `"hardware"`).
- Always ship a `config.txt.example` showing all keys with descriptive placeholder values.
- If config is missing or invalid on first boot, immediately enter Config Mode (web portal AP).

---

## 4. Web Configuration Portal

Every project includes a browser-based configuration portal served from the device over Wi-Fi.

### Behaviour

| Condition | Action |
|-----------|--------|
| First boot / no config | Start portal in **AP mode** (`M5-<ProjectName>`) with no password |
| Valid config, user requests config | Connect to STA, start portal in **STA mode** |
| Portal idle > 5 minutes | Auto-close |
| Requires credentials | `admin_user` / `admin_pass` from config |

### WebPortal Class Pattern

```cpp
class WebPortal {
public:
    WebPortal(ConfigManager* configMgr);
    void begin(bool apMode = true);
    void handle();          // Call every loop()
    void stop();
    bool isActive()     { return active; }
    void resetIdleTimer();
    bool hasTimedOut();

private:
    ConfigManager* configManager;
    WebServer server;
    bool active;
    unsigned long lastActivity;
    const unsigned long TIMEOUT_MS = 300000; // 5 minutes

    void handleRoot();       // GET  / — display HTML form
    void handleSave();       // POST /save — apply and persist config
    void handleStatus();     // GET  /status — JSON device status
    void handleNotFound();
    bool authenticate();
    String generateHTML();   // Generates the complete config page
};
```

**Rules:**
- The portal serves a **single self-contained HTML page** generated in `generateHTML()`. No external files.
- Every config field editable in `AppConfig` must have a corresponding form field.
- After saving, the device displays a "Saved — rebooting" message and calls `ESP.restart()`.
- Provide a `/status` endpoint returning JSON for programmatic checks.
- Include a debug log viewer page (`/debuglog`) if `DebugConfig::enabled` is true.

---

## 5. Display Pattern

All display output goes through the `Display` class. Never call `M5Cardputer.Display` directly from `main.cpp` or feature modules.

### Display Class Pattern

```cpp
class Display {
public:
    Display();
    void begin();
    void showStartup();          // Immediate static splash on power-on
    void showAnimatedIntro();    // Optional fancy boot animation (config-gated)
    void showConfigMode();       // Shown while web portal is active
    void showMonitoring();       // Main operational view
    void nextView();             // Cycle through display views
    DisplayView getCurrentView();
    void clearScreen();

private:
    DisplayView currentView;
    void drawHeader(const String& title);    // Blue bar, white text, top of screen
    void drawFooter();                        // Status bar at bottom
    String formatTime(time_t timestamp);
    String formatDateTime(time_t timestamp);
};
```

### Initialization (always in `begin()`)

```cpp
void Display::begin() {
    M5Cardputer.Display.setRotation(1);   // Landscape
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    clearScreen();
}
```

### Header / Footer Conventions

```cpp
// Header: filled blue bar across full width, white text
M5Cardputer.Display.fillRect(0, 0, 240, 20, BLUE);
M5Cardputer.Display.setTextColor(WHITE, BLUE);
M5Cardputer.Display.setCursor(5, 5);
M5Cardputer.Display.print(title);
M5Cardputer.Display.setTextColor(WHITE, BLACK);
```

- Screen resolution: **240 × 135** (landscape).
- `showStartup()` is called **before** SD and config loading — keep it simple (no SD access).
- `showAnimatedIntro()` is called **after** config loads and only when `hardware.fancy_intro` is `true`.
- Multiple named views cycle with `nextView()`. Use an `enum DisplayView` to track state.

---

## 6. Sound & Alert Pattern

Sound and LED alerts are managed exclusively by the `AlertManager` class.

### Hardware Pin Constants (M5 Cardputer)

```cpp
#define BUZZER_PIN     2
#define LED_PIN        21
#define NUM_LEDS       1
#define LED_BRIGHTNESS 64
```

### AlertManager Class Pattern

```cpp
class AlertManager {
public:
    AlertManager(HardwareConfig& config);
    void begin();
    void triggerAlert();    // Sound buzzer + set LED red
    void update();          // Call every loop() to handle timed shutoff
    bool isAlerting()       { return alertActive; }
    void setBuzzer(bool state);
    void setLED(uint32_t color);

    // Named status LED states
    void setStatusConnecting();
    void setStatusSyncing();
    void setStatusScanning();
    void setStatusReady();

private:
    HardwareConfig& hwConfig;
    bool alertActive;
    unsigned long alertStartTime;
};
```

### Sound (Speaker)

```cpp
// Play tone — frequency from HardwareConfig
M5Cardputer.Speaker.tone(hwConfig.buzzer_freq);

// Stop tone
M5Cardputer.Speaker.end();
```

### LED (SK6812 via FastLED)

```cpp
#include <FastLED.h>
CRGB leds[NUM_LEDS];

// In begin():
FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
FastLED.setBrightness(LED_BRIGHTNESS);
```

**Rules:**
- Buzzer frequency and duration come from `HardwareConfig` — never hardcode them.
- `update()` must be called every `loop()` to handle timed deactivation.
- Named LED states (`setStatusConnecting()` etc.) use distinct colours for user feedback.
- Screen brightness is set in `AlertManager::begin()` from `hwConfig.screen_brightness`.

---

## 7. Logging Pattern

A global `logger` instance is declared in `Logger.h` and defined in `Logger.cpp`.

```cpp
// Logger.h
extern Logger logger;
```

All modules include `Logger.h` and write through `logger.debugPrintln()`.
Debug output always goes to Serial; it additionally writes to `/<appname>/debug.log` on SD when `DebugConfig::enabled` is `true`.

### Logger Class Pattern

```cpp
class Logger {
public:
    Logger();
    bool begin();
    void setConfig(AppConfig* cfg);   // Call after config loads

    // Always writes to Serial; writes to file when debug.enabled
    void debugPrint(const char* msg);
    void debugPrintln(const char* msg);
    void debugPrintln(const String& msg);
    void debugPrintln();

    bool logEvent(const SomeEvent& event);   // App-specific structured event logging
    String getCurrentSessionFile();
    String getDebugLogFile();
    bool clearDebugLog();

private:
    String sessionFile;
    String debugFile;
    AppConfig* config;
};
```

**Rules:**
- Call `logger.setConfig(&config)` immediately after loading config in `setup()`.
- Session log files are created per-boot with a timestamp in the filename.
- Both the session log and the debug log live in `/<appname>/`.

---

## 8. Firmware Versioning & Output (rename_firmware.py)

Every project uses a PlatformIO post-build script that:
1. Reads the version from `custom_fw_version` in `platformio.ini`.
2. Injects it as `FIRMWARE_VERSION` C preprocessor macro.
3. After a successful build, copies `firmware.bin` to `output/<projectname>-<version>.bin`.

### platformio.ini Pattern

```ini
[env:m5stack-stamps3]
platform   = espressif32
board      = m5stack-stamps3
framework  = arduino
custom_fw_version = 0.1.0
extra_scripts = post:rename_firmware.py

lib_deps =
    m5stack/M5Cardputer@^1.0.0
    bblanchon/ArduinoJson@^6.21.3
    fastled/FastLED@^3.6.0

monitor_speed = 115200
upload_speed  = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_HAS_PSRAM
```

### rename_firmware.py Pattern

```python
Import("env")
import shutil, os

version = env.GetProjectOption("custom_fw_version", "dev")
env.Append(CPPDEFINES=[("FIRMWARE_VERSION", env.StringifyMacro(version))])

def copy_firmware(source, target, env):
    build_dir  = env.subst("$BUILD_DIR")
    src        = os.path.join(build_dir, "firmware.bin")
    output_dir = os.path.normpath(os.path.join(build_dir, "..", "..", "..", "output"))
    os.makedirs(output_dir, exist_ok=True)
    dest = os.path.join(output_dir, f"<projectname>-{version}.bin")
    if os.path.isfile(src):
        shutil.copy2(src, dest)
        print(f"\n*** Output: {dest}\n")
    else:
        print(f"\n*** source not found: {src}\n")

env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)
```

**Rules:**
- Bump `custom_fw_version` for every release using semantic versioning (`MAJOR.MINOR.PATCH`).
- The `output/` folder is always in `.gitignore`.
- `FIRMWARE_VERSION` must be printed to Serial in `setup()` and shown on the startup screen.
- Reference `FIRMWARE_VERSION` in code via the macro: `"Firmware v" FIRMWARE_VERSION`.

---

## 9. Application State Machine

`main.cpp` always implements a named state machine using an `enum AppState`.

```cpp
enum AppState {
    STATE_INIT,
    STATE_CONFIG_MODE,
    STATE_MONITOR_MODE
    // Add project-specific states here
};

AppState currentState = STATE_INIT;
```

- `setup()` drives init and transitions to either `STATE_CONFIG_MODE` or the operational state.
- `loop()` dispatches to a `handle<StateName>()` function per state.
- State-transition functions are named `enter<StateName>()` and contain all entry-side effects.

---

## 10. Documentation Requirements

Every project ships complete documentation in `docs/`. Each file has a mandatory structure.

| File | Contents |
|------|----------|
| `index.md` | Overview, feature table, architecture diagram (ASCII), how it works |
| `getting-started.md` | Prerequisites table, step-by-step install, first boot walkthrough |
| `configuration.md` | Every config key documented: name, type, default, description |
| `web-interface.md` | Screenshots (ASCII art acceptable), field descriptions, workflow |
| `operation.md` | Display views, controls, LED/buzzer meaning, normal use |
| `troubleshooting.md` | Symptom → cause → fix table for common problems |

**Rules:**
- All tables use standard Markdown pipe syntax.
- Config keys in docs always match the exact JSON key names in code.
- `getting-started.md` must be usable by someone with no prior knowledge of the project.
- `README.md` in the project root provides a one-screen summary and links to `docs/index.md`.

---

## 11. General Coding Rules

- **C++ standard**: Use C++11 features freely; avoid C++17 features not supported by the ESP32 Arduino framework.
- **No magic numbers**: All pin numbers, timeouts, sizes, and thresholds must be named constants defined in `Config.h` or at the top of the relevant `.cpp` file.
- **Include guards**: All headers use `#ifndef / #define / #endif` guards — never `#pragma once`.
- **Global singletons**: Use a single global instance pattern for `logger`. All other objects are instantiated in `main.cpp` and passed by pointer or reference.
- **Error handling**: SD, Wi-Fi, and NTP failures display a user-readable message and either halt (critical) or degrade gracefully (non-critical).
- **Non-blocking loop**: Never use `delay()` inside `loop()`. Use `millis()` timers for all periodic tasks.
- **Memory**: Prefer `String` for display/config text. Use `const char*` for string literals passed between functions.
