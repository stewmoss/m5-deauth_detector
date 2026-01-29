#include "APIReporter.h"

APIReporter::APIReporter(APIConfig& config) : apiConfig(config) {}

bool APIReporter::sendBatch(const std::vector<DeauthEvent>& events) {
    if (events.empty()) {
        Serial.println("No events to report");
        return true;
    }
    
    if (apiConfig.endpoint_url.isEmpty()) {
        Serial.println("API endpoint not configured");
        return false;
    }
    
    HTTPClient http;
    http.begin(apiConfig.endpoint_url);
    http.addHeader("Content-Type", "application/json");
    
    // Add custom header if configured
    if (!apiConfig.custom_header_name.isEmpty() && !apiConfig.custom_header_value.isEmpty()) {
        http.addHeader(apiConfig.custom_header_name, apiConfig.custom_header_value);
    }
    
    String payload = buildPayload(events);
    
    Serial.print("Sending ");
    Serial.print(events.size());
    Serial.println(" events to API...");
    Serial.println(payload);
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
        Serial.print("API response code: ");
        Serial.println(httpResponseCode);
        
        String response = http.getString();
        Serial.println("Response: " + response);
        
        http.end();
        return (httpResponseCode >= 200 && httpResponseCode < 300);
    } else {
        Serial.print("Error sending to API: ");
        Serial.println(http.errorToString(httpResponseCode));
        http.end();
        return false;
    }
}

String APIReporter::buildPayload(const std::vector<DeauthEvent>& events) {
    DynamicJsonDocument doc(4096);
    JsonArray array = doc.to<JsonArray>();
    
    for (const DeauthEvent& event : events) {
        JsonObject obj = array.createNestedObject();
        
        // Format timestamp as ISO 8601
        char timestamp[32];
        struct tm timeinfo;
        localtime_r(&event.timestamp, &timeinfo);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        
        obj["timestamp"] = timestamp;
        obj["target_ssid"] = event.target_ssid;
        obj["target_bssid"] = event.target_bssid;
        obj["attacker_mac"] = event.attacker_mac;
        obj["channel"] = event.channel;
        obj["rssi"] = event.rssi;
        obj["packet_count"] = event.packet_count;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}
