#include "DebugManager.h"
#include "WiFiManager.h"
#include "SharedFunctions.h"

#define DEFAULT_AP_SSID "ESP32-Portal"
#define DEFAULT_AP_PASS "12345678"
#define MAX_WIFI_ATTEMPTS 30

WiFiManager::WiFiManager()
  : server(80),
  local_IP(192, 168, 4, 1),
  gateway(192, 168, 4, 1),
  subnet(255, 255, 255, 0),
  
  manualLocal_IP(192, 168, 100, 64),
  manualGateway(192, 168, 100, 1),
  manualSubnet(255, 255, 255, 0),
  primaryDNS(8, 8, 8, 8), // Google DNS
  secondaryDNS(8, 8, 4, 4) // Google DNS
{
  Serial.begin(115200);
}

bool WiFiManager::start() {

  DEBUG_PRINTLN("Initializing Preferences...");
  EEPROM.begin(EEPROM_SIZE);
  
  ssid = sharedFunctions.readStringFromEEPROM(SSID_ADDR);
  DEBUG_PRINTF("Retrieved SSID: %s \n", ssid.c_str());

  password = sharedFunctions.readStringFromEEPROM(PASSWORD_ADDR);
  DEBUG_PRINTF("Retrieved Password: %s \n", password.c_str());
  EEPROM.end();

  if (ssid.isEmpty() || password.isEmpty()) {
    startCaptivePortal();  // No stored WiFi credentials, start Captive Portal
    return false;
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // Prevent WiFi sleep issues on ESP12-E
//  WiFi.config(manualLocal_IP, manualGateway, manualSubnet, primaryDNS, secondaryDNS);
  WiFi.begin(ssid.c_str(), password.c_str());

  return reconnect();
}

bool WiFiManager::reconnect(){
  
  DEBUG_PRINTLN("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_ATTEMPTS) {
    sharedFunctions.setRGBColor(255, 0, 0);  // red
    delay(500);
    sharedFunctions.setRGBColor(0, 0, 0);  // black
    delay(500);
    DEBUG_PRINT(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTF("\nWiFi Connected: %s \n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    DEBUG_PRINTLN("\nWiFi Connection Failed. Clearing stored credentials and restarting...");
    sharedFunctions.resetAndRestart();
    return false;  // This line won't execute as ESP restarts
  }
  
}

void WiFiManager::startCaptivePortal() {
  DEBUG_PRINTLN("Starting Captive Portal...");

  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("LittleFS Mount Failed");
    return;
  }

  WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.onNotFound([this]() { handleRedirect(); });
  server.on("/", [this]() { serveIndex(); });
  server.on("/device", HTTP_POST, [this]() { serveDevice(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.on("/scan", [this]() { handleScan(); });

  server.begin();
  DEBUG_PRINTF("Connect to: %s and enter credentials. \n", DEFAULT_AP_SSID);

}

void WiFiManager::saveCredentials(const char* deviceName, const char* accessToken) {
  //Save WiFi Credentials
  EEPROM.begin(EEPROM_SIZE);
  sharedFunctions.writeStringToEEPROM(SSID_ADDR, ssid);
  sharedFunctions.writeStringToEEPROM(PASSWORD_ADDR, password);
  //Save MQTT Credentials
  sharedFunctions.writeStringToEEPROM(DEVICE_NAME_ADDR, deviceName);
  sharedFunctions.writeStringToEEPROM(ACCESS_TOKEN_ADDR, accessToken);
  EEPROM.commit();
  EEPROM.end();
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::loop(){
  dnsServer.processNextRequest();
  server.handleClient();  
}

/**************************************Captive Portal Functions*******************************/
// Redirect all requests to portal
void WiFiManager::handleRedirect() {
  server.sendHeader("Location", "http://192.168.4.1", true);
  server.send(302, "text/plain", "");
}

void WiFiManager::serveIndex() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to load index.html");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void WiFiManager::serveDevice() {
  
  if(server.hasArg("ssid") && server.hasArg("password")){
    ssid = server.arg("ssid");
    password = server.arg("password");

    DEBUG_PRINTF("SSID: %s \n", ssid.c_str());
    DEBUG_PRINTF("Password: %s \n", password.c_str());
  }else{
    server.send(404, "text/plain", "File Not Found");
  }
  
  File file = LittleFS.open("/device.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to load device.html");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void WiFiManager::handleScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i) json += ",";
    json += "\"" + WiFi.SSID(i) + "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Save Data
void WiFiManager::handleSave() {
  if (server.hasArg("device_name") && server.hasArg("accessToken")) {
    
    String deviceName = server.arg("device_name");
    String accessToken = server.arg("accessToken");

    DEBUG_PRINTF("Device Name: %s \n", deviceName.c_str());
    DEBUG_PRINTF("Access Token: %s \n", accessToken.c_str());

    saveCredentials(deviceName.c_str(), accessToken.c_str());

    server.send(400, "text/html",
      "<html><head>"
      "<style>"
      "html, body { height: 100vh; margin: 0; display: flex; justify-content: center; align-items: center; overflow: hidden; }"
      "h3 { font-family: Arial, sans-serif; text-align: center; }"
      "</style>"
      "</head><body>"
      "<h3>Configuration Done! Restarting...</h3>"
      "</body></html>"
    );
    DEBUG_PRINTLN("Credentials saved. Restarting...");
    delay(2000);
    ESP.restart();
  } else {
    server.send(404, "text/plain", "<h3>Error: Missing Fields</h3>");
  }
}
