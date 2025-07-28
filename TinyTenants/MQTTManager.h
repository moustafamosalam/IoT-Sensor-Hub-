#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#define MQTT_MAX_PACKET_SIZE 1024

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>  // JSON Library

class MQTTManager {
    private:
        String accessToken;
        String deviceName;
        WiFiClientSecure secureClient;
        PubSubClient mqttClient;

    public:
        MQTTManager();
        bool start(void (*mqttCallback)(char*, uint8_t*, unsigned int));
        bool isConnected();
        bool reconnect();
        void publish(const char* topic, const char* payload);
        void subscribe(const char* topic);
        void disconnect();
        void loop();
        
        String getDeviceName();
        String getMQTTAccessToken();
};

#endif
