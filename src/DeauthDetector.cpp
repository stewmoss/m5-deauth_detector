#include "DeauthDetector.h"
#include "Logger.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <WiFi.h>
#include <map>

// Static instance for callback
static DeauthDetector* detectorInstance = nullptr;
static std::map<String, int> ssidPacketCounts;

// Management frame structure
typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6]; // receiver address
    uint8_t addr2[6]; // sender address
    uint8_t addr3[6]; // filtering address
    unsigned sequence_ctrl:16;
    uint8_t addr4[6]; // optional
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} wifi_ieee80211_packet_t;

DeauthDetector::DeauthDetector() : monitoring(false) {}

void DeauthDetector::begin(const std::vector<String>& protected_ssids) {
    protectedSSIDs = protected_ssids;
    detectorInstance = this;
    
    // Discover which channels the protected SSIDs are on
    discoverChannels();
}

void DeauthDetector::discoverChannels() {
    logger.debugPrintln("Discovering channels for protected SSIDs...");
    activeChannels.clear();
    ssidChannelMap.clear();
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Note: LED control will be added from main.cpp before calling this method
    
    for (int channel = 1; channel <= 14; channel++) {
        int n = WiFi.scanNetworks(false, true, false, 100, channel);
        
        for (int i = 0; i < n; i++) {
            String ssid = WiFi.SSID(i);
            for (const String& protected_ssid : protectedSSIDs) {
                if (ssid == protected_ssid) {
                    // Store the SSID to channel mapping
                    ssidChannelMap[ssid] = channel;
                    
                    bool found = false;
                    for (int ch : activeChannels) {
                        if (ch == channel) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        activeChannels.push_back(channel);
                    }
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Found '%s' on channel %d", ssid.c_str(), channel);
                    logger.debugPrintln(buf);
                }
            }
        }
    }
    
    if (activeChannels.empty()) {
        logger.debugPrintln("Warning: No protected SSIDs found. Monitoring all channels.");
        for (int i = 1; i <= 14; i++) {
            activeChannels.push_back(i);
        }
    }
    
    String channelList = "Active channels: ";
    for (int ch : activeChannels) {
        channelList += String(ch) + " ";
    }
    logger.debugPrintln(channelList);
}

void DeauthDetector::startMonitoring() {
    if (monitoring) return;
    
    logger.debugPrintln("Starting packet monitoring...");
    
    WiFi.disconnect();
    delay(100);
    
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&DeauthDetector::packetHandler);
    
    monitoring = true;
}

void DeauthDetector::stopMonitoring() {
    if (!monitoring) return;
    
    logger.debugPrintln("Stopping packet monitoring...");
    
    esp_wifi_set_promiscuous(false);
    monitoring = false;
}

void DeauthDetector::packetHandler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!detectorInstance || type != WIFI_PKT_MGMT) return;
    
    const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const wifi_ieee80211_packet_t* ipkt = (wifi_ieee80211_packet_t*)pkt->payload;
    const wifi_ieee80211_mac_hdr_t* hdr = &ipkt->hdr;
    
    // Check if it's a deauth frame (subtype 0x0C)
    uint8_t frameType = (hdr->frame_ctrl & 0xFF);
    uint8_t frameSubtype = (hdr->frame_ctrl >> 4) & 0x0F;
    
    if (frameType == 0x00 && frameSubtype == 0x0C) {
        // Deauthentication frame detected
        
        char bssid[18];
        snprintf(bssid, sizeof(bssid), "%02X:%02X:%02X:%02X:%02X:%02X",
                hdr->addr3[0], hdr->addr3[1], hdr->addr3[2],
                hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);
        
        char sender[18];
        snprintf(sender, sizeof(sender), "%02X:%02X:%02X:%02X:%02X:%02X",
                hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
        
        String bssidStr = String(bssid);
        String senderStr = String(sender);
        
        // Try to match with protected SSIDs
        // Note: We need to do a reverse lookup or track BSSID->SSID mapping
        // For now, we'll create an event and mark SSID as "Unknown" if we can't determine it
        
        DeauthEvent event;
        event.timestamp = time(nullptr);
        event.target_ssid = "Unknown"; // Will be filled in by BSSID lookup
        event.target_bssid = bssidStr;
        event.attacker_mac = senderStr;
        event.channel = pkt->rx_ctrl.channel;
        event.rssi = pkt->rx_ctrl.rssi;
        event.packet_count = 1;
        
        // Count packets by BSSID
        String key = bssidStr;
        ssidPacketCounts[key]++;
        event.packet_count = ssidPacketCounts[key];
        
        detectorInstance->events.push_back(event);
        
        char logBuf[128];
        snprintf(logBuf, sizeof(logBuf), "Deauth detected: BSSID=%s, Sender=%s, Ch=%d, RSSI=%d",
                 bssid, sender, event.channel, event.rssi);
        logger.debugPrintln(logBuf);
    }
}

bool DeauthDetector::hasEvents() {
    return !events.empty();
}

std::vector<DeauthEvent> DeauthDetector::getEvents() {
    return events;
}
int DeauthDetector::getChannelForSSID(const String& ssid) {
    // Check stored mapping from startup discovery
    auto it = ssidChannelMap.find(ssid);
    if (it != ssidChannelMap.end()) {
        return it->second;
    }
    
    // If not found in map, check if we have it from recent events
    for (const DeauthEvent& event : events) {
        if (event.target_ssid == ssid && event.channel > 0) {
            return event.channel;
        }
    }
    
    return 0; // Not found
}
void DeauthDetector::clearEvents() {
    events.clear();
}

int DeauthDetector::getEventCountForSSID(const String& ssid) {
    int count = 0;
    for (const DeauthEvent& event : events) {
        if (event.target_ssid == ssid) {
            count++;
        }
    }
    return count;
}

DeauthEvent DeauthDetector::getLastEventForSSID(const String& ssid) {
    for (int i = events.size() - 1; i >= 0; i--) {
        if (events[i].target_ssid == ssid) {
            return events[i];
        }
    }
    return DeauthEvent(); // Return empty event if not found
}

bool DeauthDetector::isProtectedSSID(const String& ssid) {
    for (const String& protected_ssid : protectedSSIDs) {
        if (ssid == protected_ssid) {
            return true;
        }
    }
    return false;
}
