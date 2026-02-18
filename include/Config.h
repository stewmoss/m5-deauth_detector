#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <vector>

// Detection constants
#define DEFAULT_PACKET_THRESHOLD 250
#define DEFAULT_CHANNEL_SCAN_TIME_MS 100
#define DEFAULT_CHANNEL_HOP_INTERVAL_MS 75

struct WiFiConfig {
    String sta_ssid;
    String sta_password;
    String admin_user;
    String admin_pass;
};

struct NTPConfig {
    String server;
    int timezone_offset;
    bool daylight_savings;
};

struct DetectionConfig {
    std::vector<String> protected_ssids;
    int reporting_interval_seconds;
    int packet_threshold;
    bool detect_all_deauth;
    int channel_scan_time_ms;
    int channel_hop_interval_ms;
};

struct APIConfig {
    String endpoint_url;
    String custom_header_name;
    String custom_header_value;
};

struct HardwareConfig {
    int buzzer_freq;
    int buzzer_duration_ms;
    int screen_brightness;
    bool fancy_intro;
};

struct DebugConfig {
    bool enabled;
};

struct AppConfig {
    WiFiConfig wifi;
    NTPConfig ntp;
    DetectionConfig detection;
    APIConfig api;
    HardwareConfig hardware;
    DebugConfig debug;
};

#endif
