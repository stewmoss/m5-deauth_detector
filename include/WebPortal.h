#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include "Config.h"
#include "ConfigManager.h"

class WebPortal {
public:
    WebPortal(ConfigManager* configMgr);
    void begin(bool apMode = true);
    void handle();
    void stop();
    bool isActive() { return active; }
    void resetIdleTimer();
    bool hasTimedOut();

private:
    ConfigManager* configManager;
    WebServer server;
    bool active;
    unsigned long lastActivity;
    const unsigned long TIMEOUT_MS = 300000; // 5 minutes

    void handleRoot();
    void handleSave();
    void handleStatus();
    void handleNotFound();
    void handleDebugLog();
    void handleDebugClear();
    bool authenticate();
    String generateHTML();
};

#endif
