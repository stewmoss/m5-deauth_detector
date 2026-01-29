#ifndef API_REPORTER_H
#define API_REPORTER_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DeauthDetector.h"
#include "Config.h"

class APIReporter {
public:
    APIReporter(APIConfig& config);
    bool sendBatch(const std::vector<DeauthEvent>& events);

private:
    APIConfig& apiConfig;
    String buildPayload(const std::vector<DeauthEvent>& events);
};

#endif
