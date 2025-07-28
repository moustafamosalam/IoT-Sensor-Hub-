#include "DebugManager.h"
#include "IRManager.h"

#define IR_LED_PIN  10 // Change this based on your connection
#define USECPERTICK 50 // microseconds per clock interrupt tick

IRManager::IRManager() : irLedPin(static_cast<gpio_num_t>(IR_LED_PIN)){}

void IRManager::init() {
    arraySize=0;
    rawDataArray = nullptr;
    
    DEBUG_PRINTLN("IR Transmitter Ready!");
}

void IRManager::transmitCode(String rawDataString) {
    arraySize = countWords(rawDataString); // Count numbers in the string
    rawDataArray = new uint16_t[arraySize]; // Allocate array dynamically

    if (rawDataArray != nullptr) { // Ensure memory allocation succeeded
        parseRawData(rawDataString, rawDataArray, arraySize);

        DEBUG_PRINTLN("Sending IR Signal...");
        ir_send_raw(rawDataArray, arraySize, 38000); // 38kHz frequency
        delete[] rawDataArray; // Free memory after use
        rawDataArray = nullptr; // Prevent accidental reuse
        DEBUG_PRINTLN("IR Signal Sent!");
    } else {
        DEBUG_PRINTLN("Memory allocation failed!");
    }
}

void IRManager::ir_send_raw(const uint16_t* data, size_t length, uint32_t frequency_hz) {
    rmt_channel_handle_t tx_channel = NULL;
    rmt_encoder_handle_t copy_encoder = NULL;
    
    // Configure RMT transmitter with correct field order
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = irLedPin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
        .flags = 0
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &tx_channel));
    
    // Create copy encoder
    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder));
    
    // Enable the channel
    ESP_ERROR_CHECK(rmt_enable(tx_channel));
    
    // Prepare the RMT symbols
    rmt_symbol_word_t* rmt_symbols = (rmt_symbol_word_t*)malloc(length * sizeof(rmt_symbol_word_t));
    if (!rmt_symbols) {
        DEBUG_PRINTLN("Failed to allocate RMT symbols");
        return;
    }
    
    for (size_t i = 0; i < length; i++) {
        rmt_symbols[i].duration0 = data[i];
        rmt_symbols[i].level0 = (i % 2 == 0) ? 1 : 0;
        rmt_symbols[i].duration1 = 0;
        rmt_symbols[i].level1 = 0;
    }
    
    // Configure carrier with EXACT field order from esp-idf header
    rmt_carrier_config_t carrier_cfg = {
        .frequency_hz = frequency_hz,  // Must come first
        .duty_cycle = 0.5f,           // Must be float
        .flags = {
            .polarity_active_low = 0,
            .always_on = 0
        }
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(tx_channel, &carrier_cfg));
    
    // Transmit configuration
    rmt_transmit_config_t transmit_config = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0
        }
    };
    
    ESP_ERROR_CHECK(rmt_transmit(tx_channel, copy_encoder, rmt_symbols, 
                               length * sizeof(rmt_symbol_word_t), &transmit_config));
    
    // Wait for completion
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel, portMAX_DELAY));
    
    // Cleanup
    free(rmt_symbols);
    ESP_ERROR_CHECK(rmt_disable(tx_channel));
    ESP_ERROR_CHECK(rmt_del_encoder(copy_encoder));
    ESP_ERROR_CHECK(rmt_del_channel(tx_channel));
}



// Function to parse the string and store values in an integer array
void IRManager::parseRawData(String data, uint16_t arr[], int &size) {
    char buf[1500]; // Temporary buffer (adjust if needed)
    data.toCharArray(buf, sizeof(buf)); // Convert String to char array
    
    char *token = strtok(buf, "_"); // Split by underscore
    int index = 0;
    
    while (token != NULL && index < size) { // Ensure we don't exceed array size
        arr[index++] = atoi(token); // Convert to integer and store in array
        token = strtok(NULL, "_"); // Move to next token
    }
    
    size = index; // Update size to actual number of elements parsed
    DEBUG_PRINTF("Parsed data: %d elements \n", size);
}

// Function to count words (numbers) in a string
int IRManager::countWords(String data) {
    int count = 0;
    char buf[1500]; // Temporary buffer
    data.toCharArray(buf, sizeof(buf));
    char *token = strtok(buf, "_"); // Split by space

    while (token != NULL) {
        count++;
        token = strtok(NULL, "_");
    }
    return count;
}
