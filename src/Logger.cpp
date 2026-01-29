#include "Logger.h"
#include <time.h>

Logger::Logger() : sessionFile("") {}

bool Logger::begin() {
    if (!SD.exists("/logs")) {
        if (!SD.mkdir("/logs")) {
            Serial.println("Failed to create logs directory");
            return false;
        }
    }
    return createSessionFile();
}

bool Logger::createSessionFile() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char filename[64];
    strftime(filename, sizeof(filename), "/logs/deauthdetect_session_%Y%m%d_%H%M%S.csv", &timeinfo);
    sessionFile = String(filename);
    
    File file = SD.open(sessionFile.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create session log file");
        return false;
    }
    
    // Write CSV header
    file.println("timestamp,target_ssid,target_bssid,attacker_mac,channel,rssi,packet_count");
    file.close();
    
    Serial.print("Created session log: ");
    Serial.println(sessionFile);
    return true;
}

bool Logger::logEvent(const DeauthEvent& event) {
    File file = SD.open(sessionFile.c_str(), FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open log file for writing");
        return false;
    }
    
    char timestamp[32];
    struct tm timeinfo;
    localtime_r(&event.timestamp, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    
    file.print(timestamp);
    file.print(",\"");
    file.print(event.target_ssid);
    file.print("\",\"");
    file.print(event.target_bssid);
    file.print("\",\"");
    file.print(event.attacker_mac);
    file.print("\",");
    file.print(event.channel);
    file.print(",");
    file.print(event.rssi);
    file.print(",");
    file.println(event.packet_count);
    
    file.close();
    return true;
}
