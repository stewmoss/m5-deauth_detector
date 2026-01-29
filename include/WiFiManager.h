#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "Config.h"

class WiFiManager {
public:
    WiFiManager(WiFiConfig& config);
    bool connectSTA();
    void disconnect();
    bool startAP(const char* ssid = "M5-DeauthDetector", const char* password = nullptr);
    void stopAP();
    bool isConnected();
    bool syncNTP(NTPConfig& ntpConfig);

private:
    WiFiConfig& wifiConfig;
};

#endif
