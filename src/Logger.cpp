#include "Logger.h"
#include <time.h>

// Define global logger instance
Logger logger;

Logger::Logger() : sessionFile(""), debugFile("/deauthdetector/logs/debug.log"), config(nullptr) {}

void Logger::setConfig(AppConfig* cfg) {
    config = cfg;
}

bool Logger::begin() {    
    // Create /deauthdetector directory if it doesn't exist
    if (!SD.exists("/deauthdetector")) {
        if (!SD.mkdir("/deauthdetector")) {
            Serial.println("Failed to create /deauthdetector directory");
            return false;
        }
    }
    
    // Create /deauthdetector/logs directory if it doesn't exist
    if (!SD.exists("/deauthdetector/logs")) {
        if (!SD.mkdir("/deauthdetector/logs")) {
            Serial.println("Failed to create /deauthdetector/logs directory");
            return false;
        }
    }
    
    // Create debug log file (cleared on each boot)
    if (!createDebugFile()) {
        Serial.println("Warning: Failed to create debug log file");
    }
    
    return createSessionFile();
}

bool Logger::createDebugFile() {
    // Create/truncate debug log file on startup
    File file = SD.open(debugFile.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }
    
    // Write header with boot timestamp
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    file.print("=== Debug Log Started: ");
    file.print(timestamp);
    file.println(" ===");
    file.close();
    
    return true;
}

void Logger::writeToDebugFile(const char* msg, bool newline) {
    if (!config || !config->debug.enabled) {
        return;
    }
    
    File file = SD.open(debugFile.c_str(), FILE_APPEND);
    if (file) {
        if (newline) {
            file.println(msg);
        } else {
            file.print(msg);
        }
        file.close();
    }
}

void Logger::debugPrint(const char* msg) {
    Serial.print(msg);
    writeToDebugFile(msg, false);
}

void Logger::debugPrint(const String& msg) {
    debugPrint(msg.c_str());
}

void Logger::debugPrintln(const char* msg) {
    Serial.println(msg);
    writeToDebugFile(msg, true);
}

void Logger::debugPrintln(const String& msg) {
    debugPrintln(msg.c_str());
}

void Logger::debugPrintln() {
    Serial.println();
    writeToDebugFile("", true);
}

bool Logger::clearDebugLog() {
    File file = SD.open(debugFile.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }
    
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    file.print("=== Debug Log Cleared: ");
    file.print(timestamp);
    file.println(" ===");
    file.close();
    
    return true;
}

bool Logger::createSessionFile() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char filename[64];
    strftime(filename, sizeof(filename), "/deauthdetector/logs/deauthdetect_session_%Y%m%d_%H%M%S.csv", &timeinfo);
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
