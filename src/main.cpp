#include <M5Cardputer.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
#include "DeauthDetector.h"
#include "Display.h"
#include "WebPortal.h"
#include "Logger.h"
#include "APIReporter.h"
#include "AlertManager.h"

// Application state
enum AppState {
    STATE_INIT,
    STATE_CONFIG_MODE,
    STATE_MONITOR_MODE
};

// Global objects
ConfigManager configManager;
WiFiManager* wifiManager = nullptr;
DeauthDetector detector;
Display display;
WebPortal* webPortal = nullptr;
APIReporter* apiReporter = nullptr;
AlertManager* alertManager = nullptr;

AppState currentState = STATE_INIT;
unsigned long lastReportTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long goButtonPressTime = 0;
bool goButtonPressed = false;
size_t lastEventCount = 0;

// Define the specific pins used by the M5Cardputer for the SD card

#define SD_SPI_SCK_PIN 40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN 12

// Forward declarations
void enterConfigMode();
void enterMonitorMode();
void handleConfigMode();
void handleMonitorMode();
void updateDisplay();

void setup() {
    // Initialize M5Cardputer
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    
    Serial.begin(115200);
    Serial.println("\n\n=== M5 Cardputer Deauth Detector ===");    

    // Initialize display
    display.begin();
    display.showStartup();  // Show basic startup first, animated intro after config loads
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

    // Initialize SD card
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        Serial.println("ERROR: SD Card initialization failed!");
        M5Cardputer.Display.fillScreen(RED);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.println("SD CARD ERROR!");
        while (1) delay(1000);
    }
    Serial.println("SD Card initialized");

    // Create /deauthdetector directory if it doesn't exist
    if (!SD.exists("/deauthdetector")) {
        if (!SD.mkdir("/deauthdetector")) {
            Serial.println("Failed to create /deauthdetector directory");
            M5Cardputer.Display.println("Cant create /deauthdetector");
            while (1) delay(1000);
        }
    }
    
    // Load configuration
    if (!configManager.loadConfig()) {
        Serial.println("Configuration not found or invalid");
        enterConfigMode();
        return;
    }
    
    // Get configuration
    AppConfig& config = configManager.getConfig();
    
    // Show animated intro or simple startup based on config
    if (config.hardware.fancy_intro) {
        display.showAnimatedIntro();
    } else {
        delay(200);  // Simple startup already shown, just wait
    }
    
    // Set config for logger (enables debug file logging if configured)
    logger.setConfig(&config);
    // Initialize logger
    if (!logger.begin()) {
        logger.debugPrintln("Warning: Logger initialization failed");
    }
    
    // Initialize managers
    alertManager = new AlertManager(config.hardware);
    alertManager->begin();
    wifiManager = new WiFiManager(config.wifi);
    apiReporter = new APIReporter(config.api);    
    
    // Connect to WiFi for time sync
    display.clearScreen();    
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.println("Connecting to WiFi... "+config.wifi.sta_ssid);
    
    // Set LED to yellow during WiFi connection
    alertManager->setStatusConnecting();
    
    if (wifiManager->connectSTA()) {
        // Sync time
        M5Cardputer.Display.println("Syncing time...");
        alertManager->setStatusSyncing();
        wifiManager->syncNTP(config.ntp);
        
        // Turn off LED after successful time sync
        alertManager->setStatusReady();
        
        // Disconnect from WiFi
        wifiManager->disconnect();
        M5Cardputer.Display.println("Disconnected");
    } else {
        logger.debugPrintln("Warning: Could not connect to WiFi for time sync");
        alertManager->setStatusReady();
    }

    // Initialize detector with LED scanning indicator
    alertManager->setStatusScanning();    
    delay(1000);
    
  
    

    detector.begin(config.detection.protected_ssids, config.detection);
    alertManager->setStatusReady();
    
    // Enter monitor mode
    enterMonitorMode();
}

void loop() {
    M5Cardputer.update();
    
    switch (currentState) {
        case STATE_CONFIG_MODE:
            handleConfigMode();
            break;
            
        case STATE_MONITOR_MODE:
            handleMonitorMode();
            break;
            
        default:
            break;
    }
}

void enterConfigMode() {
    logger.debugPrintln("Entering Config Mode");
    currentState = STATE_CONFIG_MODE;
    
    // Stop monitoring if active
    detector.stopMonitoring();
    
    // Start AP mode
    wifiManager->startAP("M5-DeauthDetector");
    
    // Start web portal
    webPortal = new WebPortal(&configManager);
    webPortal->begin(true);
    
    // Update display
    display.showConfigMode();
}

void enterMonitorMode() {
    logger.debugPrintln("Entering Monitor Mode");
    currentState = STATE_MONITOR_MODE;
    
    // Stop web portal if active
    if (webPortal) {
        webPortal->stop();
        delete webPortal;
        webPortal = nullptr;
    }
    
    // Stop AP mode
    if (wifiManager) {
        wifiManager->stopAP();
    }
    
    // Start monitoring
    detector.startMonitoring();
    
    // Update display and wait 5 seconds or until Enter is pressed
    display.showMonitoring();
    
    unsigned long displayStart = millis();
    bool enterPressed = false;
    
    while ((millis() - displayStart < 5000) && !enterPressed) {
        M5Cardputer.update();
        
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            if (status.enter) {
                enterPressed = true;
                logger.debugPrintln("Enter pressed - skipping monitoring display");
            }
        }
        
        delay(50); // Small delay to prevent excessive CPU usage
    }
    
    lastReportTime = millis();
    lastDisplayUpdate = millis();
}

void handleConfigMode() {
    // Handle web portal
    if (webPortal) {
        webPortal->handle();
        
        // Check for timeout
        if (webPortal->hasTimedOut()) {
            logger.debugPrintln("Config mode timeout - returning to monitor mode");
            enterMonitorMode();
        }
    }
    
    // Check for Enter key to exit config mode
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        if (status.enter) {
            enterMonitorMode();
        }
    }
}

void handleMonitorMode() {
    AppConfig& config = configManager.getConfig();
    
    // Update channel hopping
    detector.updateChannelHop();
    
    // Update alert manager
    if (alertManager) {
        alertManager->update();
    }
    
    // Check for new deauth events
    std::vector<DeauthEvent> events = detector.getEvents();
    
    if (!events.empty()) {
        // Trigger alert whenever new events arrived since last check
        if (alertManager && events.size() > lastEventCount) {
            alertManager->triggerAlert();
        }
        lastEventCount = events.size();
        
        // Log events
        for (const DeauthEvent& event : events) {
            logger.logEvent(event);
        }
    }
    
    // Handle reporting interval
    unsigned long currentTime = millis();
    if (currentTime - lastReportTime >= (config.detection.reporting_interval_seconds * 1000)) {
        if (detector.hasEvents()) {
            std::vector<DeauthEvent> reportEvents = detector.getEvents();
            
            // Stop monitoring temporarily
            detector.stopMonitoring();
            
            // Connect to WiFi
            if (wifiManager->connectSTA()) {
                // Send to API
                if (apiReporter) {
                    apiReporter->sendBatch(reportEvents);
                }
                
                // Disconnect
                wifiManager->disconnect();
            }
            
            // Clear events after reporting
            detector.clearEvents();
            lastEventCount = 0;
            
            // Resume monitoring
            detector.startMonitoring();
        }
        
        lastReportTime = currentTime;
    }
    
    // Update display periodically
    if (currentTime - lastDisplayUpdate >= 1000) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }
    
    // Check for keyboard input
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        
        // Enter key to cycle views
        if (status.enter) {
            display.nextView();
            updateDisplay();
        }
        
        // Left/Right keys for detailed view navigation
        if (display.getCurrentView() == VIEW_DETAILED) {
            // Check for left arrow (n) or right arrow (m)
            if (!status.word.empty()) {
                String keyStr = "";
                for (char c : status.word) {
                    keyStr += c;
                }
                
                if (keyStr == "n") { // Left arrow
                    display.prevDetailedPage(config.detection.protected_ssids.size());
                    updateDisplay();
                } else if (keyStr == "m") { // Right arrow
                    display.nextDetailedPage(config.detection.protected_ssids.size());
                    updateDisplay();
                }
            }
        }
    }
    
    // Check for Go button hold (config mode)
    if (digitalRead(0) == LOW) { // G0 button
        if (!goButtonPressed) {
            goButtonPressed = true;
            goButtonPressTime = millis();
        } else {
            // Check if held for 2 seconds
            if (millis() - goButtonPressTime >= 2000) {
                enterConfigMode();
                goButtonPressed = false;
            }
        }
    } else {
        goButtonPressed = false;
    }
}

void updateDisplay() {
    AppConfig& config = configManager.getConfig();
    std::vector<DeauthEvent> events = detector.getEvents();
    
    switch (display.getCurrentView()) {
        case VIEW_DASHBOARD:
            display.showDashboard(config.detection.protected_ssids, detector);
            break;
            
        case VIEW_LIVE_LOG:
            display.showLiveLog(events);
            break;
            
        case VIEW_DETAILED:
            display.showDetailed(config.detection.protected_ssids, detector);
            break;
    }
}
