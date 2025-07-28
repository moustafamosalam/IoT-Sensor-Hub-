#include "DebugManager.h"
#include "MQTTManager.h"
#include "SharedFunctions.h"

#define MAX_MQTT_ATTEMPTS 30
#define MQTT_BROKER "eu.thingsboard.cloud"
#define MQTT_PORT 8883

const char* fingerprint = "C8 68 62 65 90 EC E2 C4 68 D7 CF 62 F3 43 E8 BA A5 3F 96 CE";
const char* remoteCodeSubTopic = "v1/devices/me/rpc/request/+";
const char* otaTopic = "v1/devices/me/attributes";

MQTTManager::MQTTManager() : mqttClient(secureClient) {}

bool MQTTManager::start(void (*mqttCallback)(char*, uint8_t*, unsigned int)) {

  EEPROM.begin(EEPROM_SIZE);
  accessToken = sharedFunctions.readStringFromEEPROM(ACCESS_TOKEN_ADDR);
  deviceName = sharedFunctions.readStringFromEEPROM(DEVICE_NAME_ADDR);
  EEPROM.end();

  DEBUG_PRINTF("Loaded Access Token: %s\n", accessToken.c_str());
  DEBUG_PRINTF("Loaded Device Name: %s\n", deviceName.c_str());

  secureClient.setFingerprint(fingerprint);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
    
  if (accessToken.isEmpty()) {
    DEBUG_PRINTLN("MQTT credentials not set. Restarting...");
    sharedFunctions.resetAndRestart();
  }

  return reconnect();
}

bool MQTTManager::reconnect(){
  
  DEBUG_PRINT("Connecting to MQTT Broker...");

  int attempts = 0;
  
  while (!isConnected() && attempts < MAX_MQTT_ATTEMPTS) {
    sharedFunctions.setRGBColor(255, 0, 0);  // Red
    DEBUG_PRINT(".");
    if (mqttClient.connect(deviceName.c_str(), accessToken.c_str(), "")) {
      DEBUG_PRINTLN("\nMQTT Connected");
      sharedFunctions.setRGBColor(0, 255, 0);  // Blue
      subscribe(remoteCodeSubTopic);
      subscribe(otaTopic);
      return true;
    }
    delay(500);
    sharedFunctions.setRGBColor(0, 0, 0);  // black
    delay(500);
    attempts++;
  }

  DEBUG_PRINT("\nMQTT Connection failed, state=");
  DEBUG_PRINTLN(mqttClient.state());
  sharedFunctions.resetAndRestart();
  return false;
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

void MQTTManager::publish(const char* topic, const char* payload) {
    if (isConnected()) {
        mqttClient.publish(topic, payload);
    } else {
      DEBUG_PRINTLN("MQTT Not Connected. Cannot Publish.");
    }
}

void MQTTManager::subscribe(const char* topic) {
    if (isConnected()) {
        mqttClient.subscribe(topic);
        DEBUG_PRINTF("\nSubscribed to topic: %s \n", topic);
    } else {
      DEBUG_PRINTLN("MQTT Not Connected. Cannot Subscribe.");
    }
}

void MQTTManager::disconnect() {
    mqttClient.disconnect();
}

void MQTTManager::loop() {
  mqttClient.loop();
}

String MQTTManager::getDeviceName() {
    return deviceName;
}
String MQTTManager::getMQTTAccessToken() {
    return accessToken;
}
