#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <LittleFS.h>

class WiFiManager {
  private:
    String ssid;
    String password;

    DNSServer dnsServer;
    ESP8266WebServer server;

    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;

    IPAddress manualLocal_IP;
    IPAddress manualGateway;
    IPAddress manualSubnet;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

    void startCaptivePortal();
    void saveCredentials(const char* deviceName, const char* accessToken);

    void handleRedirect();
    void serveIndex();
    void serveDevice();
    void handleScan();
    void handleSave();

  public:
    WiFiManager();
    bool start();
    bool isConnected();
    bool reconnect();
    void loop();
};

#endif
