#ifndef LOGGER_H
#define LOGGER_H

#include <SD.h>
#include "DeauthDetector.h"

class Logger {
public:
    Logger();
    bool begin();
    bool logEvent(const DeauthEvent& event);
    String getCurrentSessionFile() { return sessionFile; }

private:
    String sessionFile;
    bool createSessionFile();
};

#endif
