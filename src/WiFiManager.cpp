#include "WiFiManager.h"
#include "Logger.h"
#include <time.h>

WiFiManager::WiFiManager(WiFiConfig& config) : wifiConfig(config) {}

bool WiFiManager::connectSTA() {
    if (wifiConfig.sta_ssid.isEmpty()) {
        logger.debugPrintln("WiFi SSID not configured");
        return false;
    }
    
    logger.debugPrint("Connecting to WiFi: ");
    logger.debugPrintln(wifiConfig.sta_ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.sta_ssid.c_str(), wifiConfig.sta_password.c_str());
    
    int timeout = 20; // 20 seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        logger.debugPrint(".");
        timeout--;
    }
    logger.debugPrintln();
    
    if (WiFi.status() == WL_CONNECTED) {
        logger.debugPrint("Connected! IP: ");
        logger.debugPrintln(WiFi.localIP().toString());
        return true;
    } else {
        logger.debugPrintln("Connection failed");
        return false;
    }
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

bool WiFiManager::startAP(const char* ssid, const char* password) {
    WiFi.mode(WIFI_AP);
    
    bool success;
    if (password) {
        success = WiFi.softAP(ssid, password);
    } else {
        success = WiFi.softAP(ssid);
    }
    
    if (success) {
        logger.debugPrint("AP started: ");
        logger.debugPrintln(ssid);
        logger.debugPrint("IP address: ");
        logger.debugPrintln(WiFi.softAPIP().toString());
        return true;
    } else {
        logger.debugPrintln("Failed to start AP");
        return false;
    }
}

void WiFiManager::stopAP() {
    WiFi.softAPdisconnect(true);
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::syncNTP(NTPConfig& ntpConfig) {
    logger.debugPrint("Syncing time with NTP server: ");
    logger.debugPrintln(ntpConfig.server);
    
    configTime(ntpConfig.timezone_offset * 3600, 
               ntpConfig.daylight_savings ? 3600 : 0, 
               ntpConfig.server.c_str());
    
    // Wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (time(nullptr) < 100000 && ++retry < retry_count) {
        logger.debugPrint(".");
        delay(1000);
    }
    logger.debugPrintln();
    
    if (retry < retry_count) {
        time_t now = time(nullptr);
        logger.debugPrint("Time synchronized: ");
        logger.debugPrintln(ctime(&now));
        return true;
    } else {
        logger.debugPrintln("Failed to sync time");
        return false;
    }
}
