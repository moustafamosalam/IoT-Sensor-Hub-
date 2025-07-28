#include "IRManager.h"
#include "DebugManager.h"

#define IR_RECEIVE_PIN  4  // Change this based on your connection
#define USECPERTICK 50 // microseconds per clock interrupt tick

IRManager::IRManager() 
    : irrecv(IR_RECEIVE_PIN, kCaptureBufferSize, kTimeout, true)
{}

void IRManager::init() {
    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.enableIRIn();  // Start the receiver
    DEBUG_PRINTLN("IR Receiver Ready!");
}

String IRManager::receiveCode() {
//    DEBUG_PRINTLN("Waiting for IR Signal...");
    if (irrecv.decode(&results)) {
      DEBUG_PRINTLN("Received Signal");
      if (results.overflow) {
        DEBUG_PRINTF(
            "\nWARNING: IR code is too big for buffer (>= %d). "
            "This result shouldn't be trusted until this is resolved. "
            "Edit & increase kCaptureBufferSize.\n",
            kCaptureBufferSize);
      }
      String hex_code = resultToSourceCode(&results);
      DEBUG_PRINTF("\ncaptured code: %s \n", hex_code.c_str());
      return extractRawData(hex_code);
    }
//    DEBUG_PRINTLN("Error receiving IR Signal!");
    return "";
}

bool IRManager::checkForIRCode(){
  return irrecv.decode(&results);
}

String IRManager::extractRawData(String input) {
    // Find the start and end of the raw data
    int start = input.indexOf('{') + 1; // Start after '{'
    int end = input.indexOf('}');       // End at '}'
  
    // Extract the raw data substring
    String rawData = input.substring(start, end);
  
    // Remove commas and extra spaces
    rawData.replace(",", "_"); // Replace commas with spaces
    rawData.replace(" ", ""); // Replace double spaces with single spaces
  
    // Trim leading and trailing spaces
    rawData.trim();
    DEBUG_PRINTF("\nRAW DATA: %s \n", rawData.c_str());
  
    return rawData;
}
