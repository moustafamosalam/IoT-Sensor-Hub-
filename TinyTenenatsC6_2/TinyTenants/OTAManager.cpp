#include "DebugManager.h"
#include "OTAManager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

WiFiClientSecure otaClient;  // Secure client for HTTPS

OTAManager::OTAManager(){}

// Custom OTA method for ESP32-C6 (HTTPS)
void OTAManager::checkForThingsBoardOTA(String fwUrl, String ssid, String pass) {
	if( !connectToWiFi(ssid, pass) ){
		return;
	}
	
    DEBUG_PRINTF("Starting OTA from URL: %s\n", fwUrl.c_str());

    otaClient.setInsecure(); // Accept self-signed certs

    HTTPClient http;
    http.begin(otaClient, fwUrl);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        DEBUG_PRINTF("HTTP GET failed, code: %d\n", httpCode);
        uart1.println("{\"c6_ota\": failed}");
        http.end();
        return;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
      uart1.println("{\"c6_ota\": failed1}");
      DEBUG_PRINTLN("Invalid content length");
      http.end();
      return;
    }

    if (!Update.begin(contentLength)) {
      uart1.println("{\"c6_ota\": failed2}");
      DEBUG_PRINTLN("Update.begin() failed");
      http.end();
      return;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written == contentLength) {
        uart1.println("{\"c6_ota\": success}");
        DEBUG_PRINTLN("Firmware written successfully");
    } else {
        DEBUG_PRINTF("Written %d/%d bytes\n", written, contentLength);
    }

    if (Update.end()) {
        if (Update.isFinished()) {
          uart1.println("{\"c6_ota\": success2}");
            DEBUG_PRINTLN("OTA update complete, restarting...");
            delay(1000);
            ESP.restart();
        } else {
            DEBUG_PRINTLN("Update not finished?");
        }
    } else {
        DEBUG_PRINTF("Update failed: %s\n", Update.errorString());
    }

    http.end();
}

bool OTAManager::connectToWiFi(String ssid, String password)
{
	DEBUG_PRINTF("Connecting to Wi-Fi SSID: %s\n", ssid.c_str());

    // Connect to Wi-Fi
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startAttemptTime = millis();

    // Wait for connection (max 10 seconds)
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        DEBUG_PRINT(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("\n❌ Failed to connect to Wi-Fi. OTA aborted.");
        uart1.println("{\"wifi\": failed}");
        return false;
    }

    DEBUG_PRINTLN("\n✅ Wi-Fi connected. Starting OTA...");
    uart1.println("{\"wifi\": success}");
	return true;
}
