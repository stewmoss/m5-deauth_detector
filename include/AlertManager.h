#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <Arduino.h>
#include "Config.h"

class AlertManager {
public:
    AlertManager(HardwareConfig& config);
    void begin();
    void triggerAlert();
    void update();
    bool isAlerting() { return alertActive; }    
    void setBuzzer(bool state);
    void setLED(uint32_t color);
    void setStatusConnecting();
    void setStatusSyncing();
    void setStatusScanning();
    void setStatusReady();

private:
    HardwareConfig& hwConfig;
    bool alertActive;
    unsigned long alertStartTime;
    unsigned long lastPacketTime;
    unsigned long ledTimer;
    bool ledCountdownActive;
    
};

#endif
