#include "WiFiManager.h"
#include <time.h>

WiFiManager::WiFiManager(WiFiConfig& config) : wifiConfig(config) {}

bool WiFiManager::connectSTA() {
    if (wifiConfig.sta_ssid.isEmpty()) {
        Serial.println("WiFi SSID not configured");
        return false;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(wifiConfig.sta_ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiConfig.sta_ssid.c_str(), wifiConfig.sta_password.c_str());
    
    int timeout = 20; // 20 seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        Serial.print(".");
        timeout--;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Connected! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Connection failed");
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
        Serial.print("AP started: ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
        return true;
    } else {
        Serial.println("Failed to start AP");
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
    Serial.print("Syncing time with NTP server: ");
    Serial.println(ntpConfig.server);
    
    configTime(ntpConfig.timezone_offset * 3600, 
               ntpConfig.daylight_savings ? 3600 : 0, 
               ntpConfig.server.c_str());
    
    // Wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (time(nullptr) < 100000 && ++retry < retry_count) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println();
    
    if (retry < retry_count) {
        time_t now = time(nullptr);
        Serial.print("Time synchronized: ");
        Serial.println(ctime(&now));
        return true;
    } else {
        Serial.println("Failed to sync time");
        return false;
    }
}
