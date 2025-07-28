#ifndef SHARED_FUNCTIONS_H
#define SHARED_FUNCTIONS_H

#include <FastLED.h>
#include <EEPROM.h>

#define EEPROM_SIZE      128

#define SSID_ADDR        0
#define PASSWORD_ADDR    32
#define DEVICE_NAME_ADDR 64
#define ACCESS_TOKEN_ADDR 96


class SharedFunctions {
    private:
        CRGB led[1];

    public:
        SharedFunctions();
        void initRGB();
        void setRGBColor(uint8_t r, uint8_t g, uint8_t b);
        void resetAndRestart(); // Added method to reset ESP if needed
        void writeStringToEEPROM(int startAddr, const String &data);
        String readStringFromEEPROM(int startAddr);
        void clearEEPROM();
};

// Declare a global instance
extern SharedFunctions sharedFunctions;

#endif
