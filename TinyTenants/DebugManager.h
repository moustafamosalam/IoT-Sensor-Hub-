#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

    #define DEBUG_MODE 0

    #if DEBUG_MODE
        #define DEBUG_PRINT(x) Serial.print(x)
        #define DEBUG_PRINTLN(x) Serial.println(x)
        #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
    #else
        #define DEBUG_PRINT(x)
        #define DEBUG_PRINTLN(x) 
        #define DEBUG_PRINTF(...)
    #endif

#endif
