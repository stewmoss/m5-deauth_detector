#ifndef LOGGER_H
#define LOGGER_H

#include <SD.h>
#include "DeauthDetector.h"
#include "Config.h"

class Logger {
public:
    Logger();
    bool begin();
    void setConfig(AppConfig* cfg);
    bool logEvent(const DeauthEvent& event);
    String getCurrentSessionFile() { return sessionFile; }
    String getDebugLogFile() { return debugFile; }
    
    // Debug logging methods - always writes to Serial, optionally to file
    void debugPrint(const char* msg);
    void debugPrint(const String& msg);
    void debugPrintln(const char* msg);
    void debugPrintln(const String& msg);
    void debugPrintln();
    
    // Clear the debug log file
    bool clearDebugLog();

private:
    String sessionFile;
    String debugFile;
    AppConfig* config;
    bool createSessionFile();
    bool createDebugFile();
    void writeToDebugFile(const char* msg, bool newline = false);
};

// Global logger instance - use extern in other files
extern Logger logger;

#endif
