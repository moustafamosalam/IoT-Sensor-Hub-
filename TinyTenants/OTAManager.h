#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>

class OTAManager {
    private:
       
    public:
      OTAManager();
      void initArduinoOTA();
      void OTALoop();
      void checkForThingsBoardOTA(const String& jsonPayload);
      void sendC6OTA(const String& jsonPayload);
        
};

#endif
