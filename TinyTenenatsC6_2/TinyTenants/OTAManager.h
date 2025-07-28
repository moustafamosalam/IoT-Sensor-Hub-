#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <DebugManager.h>

class OTAManager {
    private:
		bool connectToWiFi(String ssid, String password);
       
    public:
      OTAManager();
      void checkForThingsBoardOTA(String fwUrl, String ssid, String pass);
        
};

#endif
