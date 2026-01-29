#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "Config.h"
#include <SD.h>
#include <ArduinoJson.h>

class ConfigManager {
public:
    ConfigManager();
    bool loadConfig(const char* filename = "/deauthdetector/deauthconfig.txt");
    bool saveConfig(const char* filename = "/deauthdetector/deauthconfig.txt");
    AppConfig& getConfig() { return config; }
    bool isValid() { return configValid; }

private:
    AppConfig config;
    bool configValid;
    void setDefaults();
};

#endif
