#ifndef DISPLAY_H
#define DISPLAY_H

#include <M5Cardputer.h>
#include "DeauthDetector.h"
#include "Config.h"

enum DisplayView {
    VIEW_DASHBOARD,
    VIEW_LIVE_LOG,
    VIEW_DETAILED
};

class Display {
public:
    Display();
    void begin();
    void showStartup();
    void showAnimatedIntro();
    void showConfigMode();
    void showMonitoring();
    void showDashboard(const std::vector<String>& ssids, DeauthDetector& detector);
    void showLiveLog(const std::vector<DeauthEvent>& events);
    void showDetailed(const std::vector<String>& ssids, DeauthDetector& detector);
    void nextView();
    void nextDetailedPage(int maxIndex);
    void prevDetailedPage(int maxIndex);
    DisplayView getCurrentView() { return currentView; }
    void clearScreen();

private:
    DisplayView currentView;
    int detailedPageIndex;
    void drawHeader(const String& title);
    void drawFooter();
    String formatTime(time_t timestamp);
    String formatDateTime(time_t timestamp);
};

#endif
