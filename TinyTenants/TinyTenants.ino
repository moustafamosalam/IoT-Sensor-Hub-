#include "DebugManager.h"
#include <Arduino.h>
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "IRManager.h"
#include "OTAManager.h"
#include "SharedFunctions.h"

WiFiManager wifiManager;
MQTTManager mqttManager;
IRManager irManager;
OTAManager otaManager;

String currentVersion = "1.0.7";

bool setup_mood = false;
const char* sensorPubTopic = "v1/devices/me/telemetry";
const char* remoteCodesPubTopic = "v1/devices/me/attributes";

void onMessageReceived(char* topic, uint8_t* payload, unsigned int length);
void checkFactoryResetButton();

#define RELAY1_PIN 12
#define RELAY2_PIN 14
void setRelay(int relayNumber, bool state) {
  digitalWrite(relayNumber==1?RELAY1_PIN:RELAY2_PIN, state);
}

/**********************************Factory Reset*****************************/
#define FACTORY_RESET_PIN 16
#define TIME_TO_RESET 10
volatile unsigned long pressStartTime = 0;
volatile bool buttonPressed = false;
/**********************************Factory Reset*****************************/

bool cancelReceiving = false;

/**********************************OTA*****************************/
String updateMessgae = "";
bool updateRequired = false;
/**********************************OTA*****************************/

void setup() {
  if(!DEBUG_MODE) Serial.swap(); // Swap Serial to pins 13 and 15
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(FACTORY_RESET_PIN, INPUT);
  irManager.init();
  sharedFunctions.initRGB();
  // Start Captive Portal to get credentials
  DEBUG_PRINTLN("Starting WiFi Manager");
  
  if (wifiManager.start()) {
    setup_mood = false;
    mqttManager.start(onMessageReceived);
    otaManager.initArduinoOTA();
    // Report current firmware version and state
    String telemetry = "{\"fw_state\": \"UPDATED\", \"fw_version\": \"" + currentVersion + "\"}";
    mqttManager.publish(sensorPubTopic, telemetry.c_str());
    DEBUG_PRINTLN("Reported firmware version: " + currentVersion);
  }
  else{
    setup_mood = true;
    sharedFunctions.setRGBColor(255, 0, 0);  // RED 
  }
}

void loop() {
  if(setup_mood)
  {
    wifiManager.loop(); 
  }
  else{
    while(true){
      if(!mqttManager.isConnected()){
        
        if(!wifiManager.isConnected()) wifiManager.reconnect(); //Reconnect to Wifi
          
        mqttManager.reconnect();  //Reconnect to MQTT
      }
      else{
        mqttManager.loop();
        otaManager.OTALoop();
        if(updateRequired) {mqttManager.disconnect(); otaManager.checkForThingsBoardOTA(updateMessgae); updateRequired = false;}
        checkFactoryResetButton();
        if(Serial.available()){
          String receivedMsg = Serial.readStringUntil('\n'); // Read until newline
          DEBUG_PRINTF("Received Msg: %s \n", receivedMsg.c_str());
          mqttManager.publish(sensorPubTopic, receivedMsg.c_str());
        }

        delay(50); // Small delay for ESP12-E stability
      }
    }   
  }
}

/*****************************MQTT CallBack Function*************************************/
void onMessageReceived(char* topic, uint8_t * payload, unsigned int len) {
    DEBUG_PRINTF("Received message on topic %s: \n", topic);

  // Convert payload to a string
  String message = "";
  for (unsigned int i = 0; i < len; i++) {
    message += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(message);  // print entire payload

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    DEBUG_PRINTF("JSON Parsing failed: %s \n", error.c_str());
    return;
  }

  // Handle OTA if firmware URL is present
  if (message.indexOf("deleted") != -1) {
    DEBUG_PRINTLN("Deleted");
    return; // optional: exit after OTA to avoid executing other commands
  }
  else if (message.indexOf("fw_title") != -1) {
    updateRequired = true;
    updateMessgae = message;
    return; // optional: exit after OTA to avoid executing other commands
  }
  else if (message.indexOf("sw_title") != -1) {
    otaManager.sendC6OTA(message);
    return; // optional: exit after OTA to avoid executing other commands
  }

  // Extract values
  const char* method = doc["method"];
  const char* params = doc["params"];

  DEBUG_PRINTLN("Parsed JSON:");
  DEBUG_PRINTF("Method: %s \n", method);
  DEBUG_PRINTF("Params: %s \n", params);

  if(strcmp(method, "receive_ir") == 0){
    DEBUG_PRINTLN("Receiving IR Code");
    String remoteCode = "";
    while(remoteCode == ""){
      remoteCode = irManager.receiveCode();
      mqttManager.loop();
      delay(100);
      if(cancelReceiving){
        cancelReceiving = false;
        break;
      }
    }
    // Create a properly formatted JSON string
    if(remoteCode.length() > 0){
      String payload = "{\"" + String(params) + "\":\"" + remoteCode + "\"}"; 
      mqttManager.publish(remoteCodesPubTopic, payload.c_str());
      remoteCode = "";
    }else DEBUG_PRINTLN("Cancel Receiving");
  }else if(strcmp(method, "cancel") == 0){
    cancelReceiving = true;
  }else if(strcmp(method, "relay") == 0){
    setRelay(params[0] - '0', params[1] == '1'); // Set relay state based on params 
  }else if(strcmp(method, "reset") == 0){
    ESP.restart();
  }else {
    Serial.write((uint8_t*)payload, len);
    Serial.write('\n'); // Add a newline for easier parsing on receiver
  }
}
/*****************************MQTT CallBack Function*************************************/

void checkFactoryResetButton() {

  if(digitalRead(FACTORY_RESET_PIN) == LOW) {
    if(!buttonPressed) {
      buttonPressed = true;
      pressStartTime = millis();
    } else if(millis() - pressStartTime > TIME_TO_RESET*1000) {
      DEBUG_PRINTLN("Factory Reset");
      sharedFunctions.resetAndRestart();
    }
  } else {
    buttonPressed = false;
  }
}
