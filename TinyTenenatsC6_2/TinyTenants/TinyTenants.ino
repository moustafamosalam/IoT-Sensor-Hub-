// #ifndef ZIGBEE_MODE_ZCZR
// #error "Zigbee coordinator mode is not selected in Tools->Zigbee mode"
// #endif

#include <HardwareSerial.h>
#include <DebugManager.h>
#include <Arduino.h>
// #include "Zigbee.h"

#include <SensorsManager.h>
#include <IRManager.h>
#include "OTAManager.h"
#include <ArduinoJson.h>

SensorsManager sensorsManager;
OTAManager otaManager;
IRManager irManager;

void sendDummySensor(); // Function prototype

static unsigned long lastUpdate = 0;
const unsigned long interval = 5000; // 5 seconds

const char* method = "";  // Global variable to store method
const char* params = "";  // Global variable to store params
const char* ssid = "";  // Global variable to store params
const char* pass = "";  // Global variable to store params
const char* fwUrl = "";  // Global variable to store params

HardwareSerial uart1(1);  // UART1 instance

void setup() {
  DEBUG_BRGIN(115200);
  DEBUG_PRINTLN("C6 Started");
  irManager.init();
  sensorsManager.initSensors();
  uart1.println("{\"c6_version\": 1.0.2}");

  // DEBUG_PRINTLN("Starting ZigBee Coordinator...");

  // Open ZigBee network for new devices (180 seconds)
  // Zigbee.setRebootOpenNetwork(180);

  // Start ZigBee as a Coordinator
  // if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
  //   DEBUG_PRINTLN("Zigbee failed to start! Rebooting...");
  //   ESP.restart();
  // }

  // DEBUG_PRINTLN("ZigBee Coordinator started successfully.");
}

// Main loop (kept empty since ZigBee runs in the background)
void loop() {

  if (uart1.available()) {
    String receivedMsg = uart1.readStringUntil('\n'); // Read until newline
    DEBUG_PRINTF("Received via UART: %s \n", receivedMsg.c_str());

    parseMessage(receivedMsg); // Parse the received message
	if(strcmp(method, "otaC6") == 0) otaManager.checkForThingsBoardOTA(fwUrl, ssid, pass);
  if(strcmp(method, "send_ir") == 0) irManager.transmitCode(String(params));
  }

  if(millis() - lastUpdate >= interval){
    sensorsManager.sendAllSensorsData(); // Send all sensor data
    lastUpdate = millis();
  }
    
}


void sendDummySensor(){
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 5000; // 5 seconds
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis;
    uart1.println( sensorsManager.getSensorData() );
  }
}

void parseMessage(String input) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    DEBUG_PRINT("JSON Parsing failed: ");
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  method = doc["method"];
  DEBUG_PRINTLN("Parsed JSON:");
  DEBUG_PRINT("Method: ");
  DEBUG_PRINTLN(method);
    
  if(strcmp(method, "otaC6") == 0){
    ssid = doc["ssid"];
    pass = doc["pass"];
    fwUrl = doc["fwUrl"];
    
    DEBUG_PRINT("SSID: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("PASS: ");
    DEBUG_PRINTLN(pass);
    DEBUG_PRINT("fwUrl: ");
    DEBUG_PRINTLN(fwUrl);
  }else{
    params = doc["params"];
    DEBUG_PRINT("Params: ");
    DEBUG_PRINTLN(params);
  }
}
