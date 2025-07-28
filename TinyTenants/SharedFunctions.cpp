#include "DebugManager.h"
#include "SharedFunctions.h"

// Define the global instance (Only One!)
SharedFunctions sharedFunctions;

#define RGB_PIN 5    // GPIO connected to WS2813E DI
#define NUM_LEDS 1   // Only one LED

SharedFunctions::SharedFunctions(){}

/***************************RGB*******************************/
void SharedFunctions::initRGB() {
    FastLED.addLeds<WS2812B, RGB_PIN, GRB>(led, NUM_LEDS); //RBG
    setRGBColor(255, 0, 0);  // RED
    FastLED.setBrightness(255);
    FastLED.show();  // Initialize all pixels to 'off'
}
void SharedFunctions::setRGBColor(uint8_t r, uint8_t g, uint8_t b) {
    led[0] = CRGB(r, g, b);
    FastLED.show();
}
/***************************RGB*******************************/

/***************************Factory Reset*******************************/   
void SharedFunctions::resetAndRestart() {
    clearEEPROM(); 
    DEBUG_PRINTLN("Stored WiFi & MQTT credentials cleared.");

    ESP.restart();
}
/***************************Factory Reset*******************************/

/********************************EEPROM*********************************/
void SharedFunctions::writeStringToEEPROM(int startAddr, const String &data) {
    for (unsigned int i = 0; i < data.length(); ++i) {
        EEPROM.write(startAddr + i, data[i]);
    }
    EEPROM.write(startAddr + data.length(), '\0'); // نهاية السطر
}

String SharedFunctions::readStringFromEEPROM(int startAddr) {
    char data[33]; // 32 + نهاية '\0'
    int len = 0;
    unsigned char k;
    k = EEPROM.read(startAddr);
    while (k != '\0' && len < 32) {
        data[len++] = k;
        k = EEPROM.read(startAddr + len);
    }
    data[len] = '\0';
    return String(data);
}
void SharedFunctions::clearEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < EEPROM_SIZE; i++) {
      EEPROM.write(i, 0);  // أو 255 لو حابب
    }
    EEPROM.commit();
    EEPROM.end();
}
/********************************EEPROM*********************************/
