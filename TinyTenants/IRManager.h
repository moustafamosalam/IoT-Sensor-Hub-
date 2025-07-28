#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRsend.h>
#include <IRutils.h>

class IRManager {
private:
    static constexpr uint16_t kTimeout = 15;
    static constexpr uint16_t kMinUnknownSize = 12;
    static constexpr uint16_t kCaptureBufferSize = 1024;

    IRrecv irrecv;
    decode_results results;

    String extractRawData(String input);

public:
    IRManager();
    void init();
    String receiveCode();
    bool checkForIRCode();
};

#endif
