#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include <Arduino.h>
#include <DebugManager.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <driver/i2s.h>
#include <math.h>

#define BUFFER_SIZE 256  // Define a buffer size

class SensorsManager {
    private:
        sensors_event_t humidity, temp;
        Adafruit_AHTX0 aht;

        HardwareSerial motionSensor; // Use UART1
        char incomingBuffer[BUFFER_SIZE];  // Buffer to store incoming data
        int bufferIndex = 0;       // Index to keep track of buffer position

        void initAHT20();
        void initMotionSensor();
        void initAGS10();
        void initACSwitchReading();
        void initMicrophone();

        void getAHT20Data();
        void getMotionSensorData();
        void getAGS10Data();
        void getACSwitchReading();
        void getLDRData();
        void getMicrophoneData();

    public:
        SensorsManager();
        void initSensors();
        String getSensorData();
        void sendAllSensorsData();
};

#endif
