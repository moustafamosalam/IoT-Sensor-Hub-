#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <driver/rmt_tx.h>
#include <driver/gpio.h>

class IRManager {
private:
    uint16_t *rawDataArray; // Pointer for dynamic allocation
    int arraySize; // Stores actual number of values
    gpio_num_t irLedPin;  // GPIO pin for IR LED

    void ir_send_raw(const uint16_t* data, size_t length, uint32_t frequency_hz);
    void parseRawData(String data, uint16_t arr[], int &size);
    int countWords(String data);

public:
    IRManager();
    void init();
    void transmitCode(String rawDataString);
};

#endif
