#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

    #include <HardwareSerial.h>

    // Declare uart1 as extern
    extern HardwareSerial uart1;
    
    #define DEBUG_MODE 1

    #if DEBUG_MODE
		#define DEBUG_BRGIN(x) Serial.begin(x)
        #define DEBUG_PRINT(x) Serial.print(x)
        #define DEBUG_PRINTLN(x) Serial.println(x)
        #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
    #else
        #define DEBUG_PRINT(x)
        #define DEBUG_PRINTLN(x) 
        #define DEBUG_PRINTF(...)
    #endif

#endif
