#pragma once

#include "Arduino.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

#define MAX_OUTPUT_VALUE 255
#define PROFILE_NAME "LED1"
#define MAX_TRIES_COUNT 5
#define SLEEP_INTERVAL 1000
#define CACHE_DURATION_MS 5000  // Cache HTTP responses for 5 seconds

struct Arguments {
    String profileName;
    int red;
    int green;
    int blue;
    int percentage;
    bool is_current;
    int secondaryRed;
    int secondaryGreen;
    int secondaryBlue;
    int secondaryRed2;
    int secondaryGreen2;
    int secondaryBlue2;
    bool enabled;
    bool useSecondary;
};

class HttpControl 
{    
public:

    HttpControl();
    bool getLedProfileParameters(String& profileName, int& red, int& green, int& blue, int& percentage);
    Arguments getLedProfileFullParameters();
    
private:

    String baseMacString;
    //String getCurrentLedProfileUrl = "http://led.haven/neon_led_control/led_profiles/get_by_name?led_profile_name=" PROFILE_NAME;
    String getCurrentLedProfileUrl = "http://led.haven/neon_led_control/led_profiles/current";
    //String getCurrentLedProfileUrl = "http://192.168.0.120:8000/neon_led_control/led_profiles/current";
    void readMacAddress();
    
    // Cache variables
    Arguments cachedArguments;
    unsigned long lastHttpRequestTime;
    bool cacheValid;
};