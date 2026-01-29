#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <vector>

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
    int silence_gap_seconds;
    int led_hold_seconds;
    int reporting_interval_seconds;
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
