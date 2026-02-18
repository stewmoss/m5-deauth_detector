#include "WebPortal.h"
#include "Logger.h"

WebPortal::WebPortal(ConfigManager* configMgr) 
    : configManager(configMgr), server(80), active(false), lastActivity(0) {}

void WebPortal::begin(bool apMode) {
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/save", HTTP_POST, [this]() { this->handleSave(); });
    server.on("/status", [this]() { this->handleStatus(); });
    server.on("/debug/log", [this]() { this->handleDebugLog(); });
    server.on("/debug/clear", HTTP_POST, [this]() { this->handleDebugClear(); });
    server.onNotFound([this]() { this->handleNotFound(); });
    
    server.begin();
    active = true;
    lastActivity = millis();
    
    logger.debugPrintln("Web portal started");
}

void WebPortal::handle() {
    if (active) {
        server.handleClient();
    }
}

void WebPortal::stop() {
    server.stop();
    active = false;
    logger.debugPrintln("Web portal stopped");
}

void WebPortal::resetIdleTimer() {
    lastActivity = millis();
}

bool WebPortal::hasTimedOut() {
    return (millis() - lastActivity) > TIMEOUT_MS;
}

bool WebPortal::authenticate() {
    AppConfig& config = configManager->getConfig();
    
    if (!server.authenticate(config.wifi.admin_user.c_str(), 
                            config.wifi.admin_pass.c_str())) {
        server.requestAuthentication();
        return false;
    }
    return true;
}

void WebPortal::handleRoot() {
    if (!authenticate()) return;
    
    resetIdleTimer();
    server.send(200, "text/html", generateHTML());
}

void WebPortal::handleSave() {
    if (!authenticate()) return;
    
    resetIdleTimer();
    
    AppConfig& config = configManager->getConfig();
    
    // Parse form data
    if (server.hasArg("sta_ssid")) {
        config.wifi.sta_ssid = server.arg("sta_ssid");
    }
    if (server.hasArg("sta_password")) {
        config.wifi.sta_password = server.arg("sta_password");
    }
    if (server.hasArg("admin_user")) {
        config.wifi.admin_user = server.arg("admin_user");
    }
    if (server.hasArg("admin_pass")) {
        config.wifi.admin_pass = server.arg("admin_pass");
    }
    
    if (server.hasArg("ntp_server")) {
        config.ntp.server = server.arg("ntp_server");
    }
    if (server.hasArg("timezone_offset")) {
        config.ntp.timezone_offset = server.arg("timezone_offset").toInt();
    }
    if (server.hasArg("daylight_savings")) {
        config.ntp.daylight_savings = server.arg("daylight_savings") == "true";
    }
    
    if (server.hasArg("protected_ssids")) {
        String ssidsStr = server.arg("protected_ssids");
        config.detection.protected_ssids.clear();
        
        // Parse comma-separated SSIDs
        int start = 0;
        int comma = ssidsStr.indexOf(',');
        while (comma != -1) {
            String ssid = ssidsStr.substring(start, comma);
            ssid.trim();
            if (!ssid.isEmpty()) {
                config.detection.protected_ssids.push_back(ssid);
            }
            start = comma + 1;
            comma = ssidsStr.indexOf(',', start);
        }
        String lastSSID = ssidsStr.substring(start);
        lastSSID.trim();
        if (!lastSSID.isEmpty()) {
            config.detection.protected_ssids.push_back(lastSSID);
        }
    }
    
    if (server.hasArg("reporting_interval")) {
        config.detection.reporting_interval_seconds = server.arg("reporting_interval").toInt();
    }
    if (server.hasArg("packet_threshold")) {
        config.detection.packet_threshold = server.arg("packet_threshold").toInt();
    }
    config.detection.detect_all_deauth = server.hasArg("detect_all_deauth");
    if (server.hasArg("channel_scan_time")) {
        config.detection.channel_scan_time_ms = server.arg("channel_scan_time").toInt();
    }
    if (server.hasArg("channel_hop_interval")) {
        config.detection.channel_hop_interval_ms = server.arg("channel_hop_interval").toInt();
    }
    
    if (server.hasArg("api_url")) {
        config.api.endpoint_url = server.arg("api_url");
    }
    if (server.hasArg("api_header_name")) {
        config.api.custom_header_name = server.arg("api_header_name");
    }
    if (server.hasArg("api_header_value")) {
        config.api.custom_header_value = server.arg("api_header_value");
    }
    
    if (server.hasArg("buzzer_freq")) {
        config.hardware.buzzer_freq = server.arg("buzzer_freq").toInt();
    }
    if (server.hasArg("buzzer_duration")) {
        config.hardware.buzzer_duration_ms = server.arg("buzzer_duration").toInt();
    }
    if (server.hasArg("screen_brightness")) {
        config.hardware.screen_brightness = server.arg("screen_brightness").toInt();
    }
    
    // Debug config - checkbox only sends value if checked
    config.debug.enabled = server.hasArg("debug_enabled");
    
    // Save to SD card
    if (configManager->saveConfig()) {
        server.send(200, "text/html", 
            "<html><body><h2>Configuration Saved!</h2>"
            "<p>Device will restart in 3 seconds...</p>"
            "<script>setTimeout(function(){window.location='/';}, 3000);</script>"
            "</body></html>");
        
        delay(3000);
        ESP.restart();
    } else {
        server.send(500, "text/html", 
            "<html><body><h2>Error</h2>"
            "<p>Failed to save configuration</p>"
            "<a href='/'>Back</a></body></html>");
    }
}

void WebPortal::handleStatus() {
    if (!authenticate()) return;
    
    resetIdleTimer();
    
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis() / 1000);
    json += "}";
    
    server.send(200, "application/json", json);
}

void WebPortal::handleNotFound() {
    // Captive portal redirect
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

void WebPortal::handleDebugLog() {
    if (!authenticate()) return;
    
    resetIdleTimer();
    
    String debugFile = logger.getDebugLogFile();
    
    if (!SD.exists(debugFile.c_str())) {
        server.send(404, "text/plain", "Debug log file not found");
        return;
    }
    
    File file = SD.open(debugFile.c_str(), FILE_READ);
    if (!file) {
        server.send(500, "text/plain", "Failed to open debug log file");
        return;
    }
    
    // Stream the file content
    server.streamFile(file, "text/plain");
    file.close();
}

void WebPortal::handleDebugClear() {
    if (!authenticate()) return;
    
    resetIdleTimer();
    
    if (logger.clearDebugLog()) {
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    } else {
        server.send(500, "text/html", 
            "<html><body><h2>Error</h2>"
            "<p>Failed to clear debug log</p>"
            "<a href='/'>Back</a></body></html>");
    }
}

String WebPortal::generateHTML() {
    AppConfig& config = configManager->getConfig();
    
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>M5 Deauth Detector Config</title>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: auto; background: white; padding: 20px; border-radius: 8px; }
        h1 { color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; }
        .tabs { overflow: hidden; border-bottom: 1px solid #ccc; margin-bottom: 20px; }
        .tab { float: left; padding: 10px 20px; cursor: pointer; background: #f0f0f0; border: 1px solid #ccc; border-bottom: none; margin-right: 5px; }
        .tab.active { background: white; border-bottom: 1px solid white; margin-bottom: -1px; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        label { display: block; margin: 10px 0 5px; font-weight: bold; }
        input[type='text'], input[type='password'], input[type='number'] { 
            width: 100%; padding: 8px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;
        }
        input[type='checkbox'] { margin-right: 5px; }
        button { 
            background: #007bff; color: white; padding: 10px 20px; 
            border: none; border-radius: 4px; cursor: pointer; margin-top: 20px;
        }
        button:hover { background: #0056b3; }
        .info { background: #e7f3ff; padding: 10px; border-left: 4px solid #007bff; margin: 10px 0; }
    </style>
    <script>
        function showTab(tabName) {
            var tabs = document.getElementsByClassName('tab-content');
            for (var i = 0; i < tabs.length; i++) {
                tabs[i].classList.remove('active');
            }
            var tabButtons = document.getElementsByClassName('tab');
            for (var i = 0; i < tabButtons.length; i++) {
                tabButtons[i].classList.remove('active');
            }
            document.getElementById(tabName).classList.add('active');
            event.target.classList.add('active');
        }
    </script>
</head>
<body>
    <div class='container'>
        <h1>M5 Deauth Detector Configuration</h1>
        
        <div class='tabs'>
            <div class='tab active' onclick='showTab("wifi")'>WiFi</div>
            <div class='tab' onclick='showTab("detection")'>Detection</div>
            <div class='tab' onclick='showTab("api")'>API</div>
            <div class='tab' onclick='showTab("hardware")'>Hardware</div>
            <div class='tab' onclick='showTab("debug")'>Debug</div>
        </div>
        
        <form method='POST' action='/save'>
            
            <div id='wifi' class='tab-content active'>
                <div class='info'>Configure WiFi connection and admin credentials</div>
                
                <label>WiFi SSID:</label>
                <input type='text' name='sta_ssid' value=')" + config.wifi.sta_ssid + R"(' required>
                
                <label>WiFi Password:</label>
                <input type='password' name='sta_password' value=')" + config.wifi.sta_password + R"('>
                
                <label>Admin Username:</label>
                <input type='text' name='admin_user' value=')" + config.wifi.admin_user + R"(' required>
                
                <label>Admin Password:</label>
                <input type='password' name='admin_pass' value=')" + config.wifi.admin_pass + R"(' required>
                
                <label>NTP Server:</label>
                <input type='text' name='ntp_server' value=')" + config.ntp.server + R"('>
                
                <label>Timezone Offset (hours):</label>
                <input type='number' name='timezone_offset' value=')" + String(config.ntp.timezone_offset) + R"('>
                
                <label>
                    <input type='checkbox' name='daylight_savings' value='true' )" + 
                    String(config.ntp.daylight_savings ? "checked" : "") + R"(>
                    Daylight Savings Time
                </label>
            </div>
            
            <div id='detection' class='tab-content'>
                <div class='info'>Configure protected SSIDs and detection parameters</div>
                
                <label>Protected SSIDs (comma-separated):</label>
                <input type='text' name='protected_ssids' value=')";
    
    // Add protected SSIDs
    for (size_t i = 0; i < config.detection.protected_ssids.size(); i++) {
        html += config.detection.protected_ssids[i];
        if (i < config.detection.protected_ssids.size() - 1) {
            html += ", ";
        }
    }
    
    html += R"(' required>
                

                <label>Reporting Interval (seconds):</label>
                <input type='number' name='reporting_interval' value=')" + String(config.detection.reporting_interval_seconds) + R"('>
                
                <label>Packet Threshold:</label>
                <input type='number' name='packet_threshold' value=')" + String(config.detection.packet_threshold) + R"(' min='1'>
                
                <label>
                    <input type='checkbox' name='detect_all_deauth' value='true' )" + 
                    String(config.detection.detect_all_deauth ? "checked" : "") + R"(>
                    Detect All Deauth (not just protected SSIDs)
                </label>
                
                <label>Channel Scan Time (milliseconds):</label>
                <input type='number' name='channel_scan_time' value=')" + String(config.detection.channel_scan_time_ms) + R"(' min='50'>
                
                <label>Channel Hop Interval (milliseconds):</label>
                <input type='number' name='channel_hop_interval' value=')" + String(config.detection.channel_hop_interval_ms) + R"(' min='75'>
            </div>
            
            <div id='api' class='tab-content'>
                <div class='info'>Configure remote API reporting</div>
                
                <label>API Endpoint URL:</label>
                <input type='text' name='api_url' value=')" + config.api.endpoint_url + R"('>
                
                <label>Custom Header Name:</label>
                <input type='text' name='api_header_name' value=')" + config.api.custom_header_name + R"('>
                
                <label>Custom Header Value:</label>
                <input type='text' name='api_header_value' value=')" + config.api.custom_header_value + R"('>
            </div>
            
            <div id='hardware' class='tab-content'>
                <div class='info'>Configure buzzer and display settings</div>
                
                <label>Buzzer Frequency (Hz):</label>
                <input type='number' name='buzzer_freq' value=')" + String(config.hardware.buzzer_freq) + R"('>
                
                <label>Buzzer Duration (ms):</label>
                <input type='number' name='buzzer_duration' value=')" + String(config.hardware.buzzer_duration_ms) + R"('>
                
                <label>Screen Brightness (0-255):</label>
                <input type='number' name='screen_brightness' value=')" + String(config.hardware.screen_brightness) + R"(' min='0' max='255'>
            </div>
            
            <div id='debug' class='tab-content'>
                <div class='info'>Configure debug logging to SD card</div>
                
                <label>
                    <input type='checkbox' name='debug_enabled' value='true' )" + 
                    String(config.debug.enabled ? "checked" : "") + R"(>
                    Enable Debug Logging
                </label>
                
                <p style='margin-top: 20px;'>
                    <a href='/debug/log' target='_blank' style='color: #007bff;'>View Debug Log</a>
                </p>
                
                <form method='POST' action='/debug/clear' style='display: inline;'>
                    <button type='submit' style='background: #dc3545; margin-top: 10px;'>Clear Debug Log</button>
                </form>
            </div>
            
            <button type='submit'>Save Configuration</button>
        </form>
    </div>
</body>
</html>
)";
    
    return html;
}
