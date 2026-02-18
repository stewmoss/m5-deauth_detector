#ifndef DEAUTH_DETECTOR_H
#define DEAUTH_DETECTOR_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "Config.h"

struct DeauthEvent {
    time_t timestamp;
    String target_ssid;
    String target_bssid;
    String attacker_mac;
    int channel;
    int rssi;
    int packet_count;
};

class DeauthDetector {
public:
    DeauthDetector();
    void begin(const std::vector<String>& protected_ssids, const DetectionConfig& config);
    void startMonitoring();
    void stopMonitoring();
    bool hasEvents();
    std::vector<DeauthEvent> getEvents();
    void clearEvents();
    int getEventCountForSSID(const String& ssid);
    DeauthEvent getLastEventForSSID(const String& ssid);
    int getChannelForSSID(const String& ssid);
    void updateChannelHop();

private:
    std::vector<String> protectedSSIDs;
    std::vector<DeauthEvent> events;
    std::vector<int> activeChannels;
    std::map<String, int> ssidChannelMap;
    bool monitoring;
    DetectionConfig detectionConfig;
    int currentChannelIndex;
    unsigned long lastChannelHopTime;
    
    void discoverChannels();
    static void packetHandler(void* buf, wifi_promiscuous_pkt_type_t type);
    bool isProtectedSSID(const String& ssid);
    bool shouldDetectDeauth();
    
    void discoverChannels();
    static void packetHandler(void* buf, wifi_promiscuous_pkt_type_t type);
    bool isProtectedSSID(const String& ssid);
};

#endif
