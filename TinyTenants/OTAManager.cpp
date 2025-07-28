#include "DebugManager.h"
#include "OTAManager.h"
#include "SharedFunctions.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>

#define OTA_PASSWORD "123456" // Set your OTA password here
WiFiClientSecure otaWifiClient;  // Secure client for HTTPS

OTAManager::OTAManager(){}

void OTAManager::initArduinoOTA(){

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    DEBUG_PRINTLN("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });

  ArduinoOTA.setHostname("TinyTenants"); // Set your device name here
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
  
}

void OTAManager::OTALoop(){
  ArduinoOTA.handle();
}

// Custom OTA method for ThingsBoard
void OTAManager::checkForThingsBoardOTA(const String& jsonPayload) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);
  if (error) {
    DEBUG_PRINTLN("Failed to parse OTA JSON");
    return;
  }

  if (doc.containsKey("fw_title")) {
    otaWifiClient.setInsecure();
    
    String fwTitle = doc["fw_title"];
    String fwVersion = doc["fw_version"];
    
    EEPROM.begin(EEPROM_SIZE);
    String accessToken = sharedFunctions.readStringFromEEPROM(ACCESS_TOKEN_ADDR);
    EEPROM.end();
    
    String fwUrl = "https://eu.thingsboard.cloud/api/v1/" + accessToken + "/firmware?title=" + fwTitle + "&version=" + fwVersion;    
    DEBUG_PRINTF("Starting OTA from URL: %s\n", fwUrl.c_str());

    sharedFunctions.setRGBColor(255, 255, 255);  // Green
    t_httpUpdate_return ret = ESPhttpUpdate.update(otaWifiClient, fwUrl);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        DEBUG_PRINTF("HTTP_UPDATE_FAILED Error (%d): %s\n",
                     ESPhttpUpdate.getLastError(),
                     ESPhttpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_NO_UPDATES:
        DEBUG_PRINTLN("HTTP_UPDATE_NO_UPDATES");
        break;
      case HTTP_UPDATE_OK:
        DEBUG_PRINTLN("HTTP_UPDATE_OK");
        break;
    }
  }
}

void OTAManager::sendC6OTA(const String& jsonPayload)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);
  if (error) {
    DEBUG_PRINTLN("Failed to parse OTA JSON");
    return;
  }

  if (doc.containsKey("sw_title")) {
    otaWifiClient.setInsecure();
    
    String swTitle = doc["sw_title"];
    String swVersion = doc["sw_version"];
    
    EEPROM.begin(EEPROM_SIZE);
    String accessToken = sharedFunctions.readStringFromEEPROM(ACCESS_TOKEN_ADDR);
    String ssid = sharedFunctions.readStringFromEEPROM(SSID_ADDR);
    String password = sharedFunctions.readStringFromEEPROM(PASSWORD_ADDR);
    EEPROM.end();
    
    String fwUrl = "https://eu.thingsboard.cloud/api/v1/" + accessToken + "/software?title=" + swTitle + "&version=" + swVersion;   

    doc.clear();
    doc["method"] = "otaC6";
    doc["ssid"] = ssid;
    doc["pass"] = password;
    doc["fwUrl"] = fwUrl;

    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println(jsonString);
    
  }
}
