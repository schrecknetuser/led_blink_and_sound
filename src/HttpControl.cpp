#include "HttpControl.h"
#include <esp_wifi.h>

void HttpControl::readMacAddress(){
    uint8_t baseMac[6];
    char macAddressCharArray[128];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK) {
        sprintf(macAddressCharArray, "%02x:%02x:%02x:%02x:%02x:%02x",
                    baseMac[0], baseMac[1], baseMac[2],
                    baseMac[3], baseMac[4], baseMac[5]);
        baseMacString = macAddressCharArray;
    } else {
        Serial.println("Failed to read MAC address");
    }
}

HttpControl::HttpControl()
{
    readMacAddress();
    lastHttpRequestTime = 0;
    cacheValid = false;
    memset(&cachedArguments, 0, sizeof(cachedArguments));
}

Arguments HttpControl::getLedProfileFullParameters()
{
    // Check if cached data is still valid
    unsigned long currentTime = millis();
    if (cacheValid && (currentTime - lastHttpRequestTime < CACHE_DURATION_MS)) {
        return cachedArguments;
    }

    HTTPClient http;
    String serverResponse;
    JsonDocument doc;
    Arguments result;
    int triesCount = 0;

    http.begin(getCurrentLedProfileUrl + "?mac_address=" + baseMacString);

    while(triesCount < MAX_TRIES_COUNT)
    {
        if(http.GET() == 200)
            break;
        Serial.println("HTTP Error - attempt " + String(triesCount + 1));
        delay(SLEEP_INTERVAL);
        yield(); // Allow other tasks to run
        triesCount++;
    }

    if(triesCount == MAX_TRIES_COUNT)
    {
        Serial.println("HTTP failed after all retries, using cached data or defaults");
        memset(&result, 0, sizeof(result));
        http.end();
        
        // If we have cached data, return it even if expired
        if (cacheValid) {
            return cachedArguments;
        }
        return result;
    }

    serverResponse = http.getString();
    deserializeJson(doc, serverResponse);

    result.profileName = ((const char*)doc["profile_name"]);
    result.percentage = (int)doc["percentage"];
    result.enabled = (bool)doc["enabled"];
    result.useSecondary = (bool)doc["use_secondary"];
    result.red = (int)doc["red"];
    result.green = (int)doc["green"];
    result.blue = (int)doc["blue"];    
    result.secondaryRed = (int)doc["secondary_red"];
    result.secondaryGreen = (int)doc["secondary_green"];
    result.secondaryBlue = (int)doc["secondary_blue"];
    result.secondaryRed2 = (int)doc["secondary_red2"];
    result.secondaryGreen2 = (int)doc["secondary_green2"];
    result.secondaryBlue2 = (int)doc["secondary_blue2"];

    http.end();

    // Update cache
    cachedArguments = result;
    lastHttpRequestTime = currentTime;
    cacheValid = true;

    return result;
}

bool HttpControl::getLedProfileParameters(String& profileName, int& red, int& green, int& blue, int& percentage)
{
    Arguments arguments = getLedProfileFullParameters();

    percentage = arguments.percentage;
    profileName = arguments.profileName;

    if(arguments.enabled == false)
    {
        red = 0;
        green = 0;
        blue = 0;
        return true;
    }

    if(arguments.useSecondary == false)
    {
        red = arguments.red;
        green = arguments.green;
        blue = arguments.blue;
    }
    else
    {
        red = arguments.secondaryRed;
        green = arguments.secondaryGreen;
        blue = arguments.secondaryBlue;
    }
    

    return true;
}
