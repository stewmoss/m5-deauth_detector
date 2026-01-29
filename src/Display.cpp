#include "Display.h"

Display::Display() : currentView(VIEW_DASHBOARD), detailedPageIndex(0) {}

void Display::begin() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    clearScreen();
}

void Display::clearScreen() {
    M5Cardputer.Display.fillScreen(BLACK);
}

void Display::drawHeader(const String& title) {
    M5Cardputer.Display.fillRect(0, 0, 240, 20, BLUE);
    M5Cardputer.Display.setTextColor(WHITE, BLUE);
    M5Cardputer.Display.setCursor(5, 5);
    M5Cardputer.Display.print(title);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
}

void Display::showStartup() {
    clearScreen();
    M5Cardputer.Display.setCursor(50, 50);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.println("Deauth");
    M5Cardputer.Display.setCursor(50, 70);
    M5Cardputer.Display.println("Detector");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(40, 100);
    M5Cardputer.Display.println("Initializing...");
    M5Cardputer.Display.setCursor(1, 110);
    M5Cardputer.Display.println("By Stewart Moss (c) 2026.");
}

void Display::showConfigMode() {
    clearScreen();
    drawHeader("Config Mode");
    
    M5Cardputer.Display.setCursor(5, 30);
    M5Cardputer.Display.println("Configuration Portal Active");
    M5Cardputer.Display.setCursor(5, 50);
    M5Cardputer.Display.println("Connect to:");
    M5Cardputer.Display.setCursor(5, 65);
    M5Cardputer.Display.println("SSID: M5-DeauthDetector");
    M5Cardputer.Display.setCursor(5, 85);
    M5Cardputer.Display.println("Navigate to:");
    M5Cardputer.Display.setCursor(5, 100);
    M5Cardputer.Display.println("http://192.168.4.1");
}

void Display::showMonitoring() {
    clearScreen();
    drawHeader("Monitoring");
    
    M5Cardputer.Display.setCursor(5, 30);
    M5Cardputer.Display.println("Monitoring for deauth attacks");
    M5Cardputer.Display.setCursor(5, 50);
    M5Cardputer.Display.println("Press ENTER to cycle views");
    M5Cardputer.Display.setCursor(5, 65);
    M5Cardputer.Display.println("Hold GO for config mode");
}

void Display::showDashboard(const std::vector<String>& ssids, DeauthDetector& detector) {
    clearScreen();
    drawHeader("Dashboard");
    
    int y = 30;
    for (const String& ssid : ssids) {
        int count = detector.getEventCountForSSID(ssid);
        M5Cardputer.Display.setCursor(5, y);
        M5Cardputer.Display.print(ssid);
        M5Cardputer.Display.setCursor(180, y);
        M5Cardputer.Display.print(count);
        y += 15;
        
        if (y > 120) break; // Screen limit
    }
    
    drawFooter();
}

void Display::showLiveLog(const std::vector<DeauthEvent>& events) {
    clearScreen();
    drawHeader("Live Log");
    
    int y = 30;
    int count = 0;
    
    // Show last 5 events
    for (int i = events.size() - 1; i >= 0 && count < 5; i--) {
        const DeauthEvent& event = events[i];
        
        M5Cardputer.Display.setCursor(5, y);
        M5Cardputer.Display.print(formatTime(event.timestamp));
        
        M5Cardputer.Display.setCursor(5, y + 10);
        M5Cardputer.Display.print("SSID: ");
        M5Cardputer.Display.print(event.target_ssid.substring(0, 12));
        
        M5Cardputer.Display.setCursor(5, y + 20);
        M5Cardputer.Display.print("Ch:");
        M5Cardputer.Display.print(event.channel);
        M5Cardputer.Display.print(" RSSI:");
        M5Cardputer.Display.print(event.rssi);
        
        y += 35;
        count++;
        
        if (y > 120) break;
    }
    
    if (events.empty()) {
        M5Cardputer.Display.setCursor(5, 60);
        M5Cardputer.Display.println("No events detected");
    }
    
    drawFooter();
}

void Display::showDetailed(const std::vector<String>& ssids, DeauthDetector& detector) {
    clearScreen();
    
    if (ssids.empty()) {
        drawHeader("Details");
        M5Cardputer.Display.setCursor(5, 60);
        M5Cardputer.Display.println("No SSIDs configured");
        drawFooter();
        return;
    }
    
    if (detailedPageIndex >= ssids.size()) {
        detailedPageIndex = 0;
    }
    
    const String& ssid = ssids[detailedPageIndex];
    
    // Draw header with page indicator
    M5Cardputer.Display.fillRect(0, 0, 240, 20, BLUE);
    M5Cardputer.Display.setTextColor(WHITE, BLUE);
    M5Cardputer.Display.setCursor(5, 5);
    M5Cardputer.Display.print("Details: " + ssid.substring(0, 10));
    
    // Show page indicator on the right side of header
    String pageIndicator = String(detailedPageIndex + 1) + "/" + String(ssids.size());
    int indicatorWidth = pageIndicator.length() * 6; // Approximate width
    M5Cardputer.Display.setCursor(240 - indicatorWidth - 5, 5);
    M5Cardputer.Display.print(pageIndicator);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    
    int count = detector.getEventCountForSSID(ssid);
    DeauthEvent lastEvent = detector.getLastEventForSSID(ssid);
    int channel = detector.getChannelForSSID(ssid);
    
    M5Cardputer.Display.setCursor(5, 30);
    M5Cardputer.Display.println("Total Packets:");
    M5Cardputer.Display.setCursor(5, 45);
    M5Cardputer.Display.print("  ");
    M5Cardputer.Display.println(count);
    
    M5Cardputer.Display.setCursor(5, 60);
    M5Cardputer.Display.println("Channel:");
    M5Cardputer.Display.setCursor(5, 75);
    M5Cardputer.Display.print("  ");
    M5Cardputer.Display.println(channel > 0 ? String(channel) : "N/A");
    
    M5Cardputer.Display.setCursor(5, 90);
    M5Cardputer.Display.println("Last Attacker MAC:");
    M5Cardputer.Display.setCursor(5, 105);
    M5Cardputer.Display.print("  ");
    M5Cardputer.Display.println(lastEvent.attacker_mac);
    
    drawFooter();
}

void Display::nextView() {
    switch (currentView) {
        case VIEW_DASHBOARD:
            currentView = VIEW_LIVE_LOG;
            break;
        case VIEW_LIVE_LOG:
            currentView = VIEW_DETAILED;
            detailedPageIndex = 0;
            break;
        case VIEW_DETAILED:
            currentView = VIEW_DASHBOARD;
            break;
    }
}

void Display::nextDetailedPage(int maxIndex) {
    if (maxIndex <= 0) return;
    detailedPageIndex++;
    if (detailedPageIndex >= maxIndex) {
        detailedPageIndex = 0;
    }
}

void Display::prevDetailedPage(int maxIndex) {
    if (maxIndex <= 0) return;
    detailedPageIndex--;
    if (detailedPageIndex < 0) {
        detailedPageIndex = maxIndex - 1;
    }
}

String Display::formatTime(time_t timestamp) {
    if (timestamp == 0) return "N/A";
    
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
}

String Display::formatDateTime(time_t timestamp) {
    if (timestamp == 0) return "N/A";
    
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}

void Display::drawFooter() {
    time_t now = time(nullptr);
    String dateTime = formatDateTime(now);
    
    // Draw footer background
    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLUE);
    M5Cardputer.Display.setTextColor(WHITE, BLUE);
    M5Cardputer.Display.setCursor(5, 127);
    M5Cardputer.Display.print(dateTime);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
}
