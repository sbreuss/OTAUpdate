# OTAUpdate
OTAUpdate is a Library to perform Over-The-Air Updates of your [LinkIt ONE](https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/whatis_linkit/index.gsp). The library can be used to check for firmware updates and install them remotely into the flash of the board. 


The update check and download is performed using HTTP connections with the included GPRS Module.

To use the OTA feature, you have to build the examples/OTA.ino sketch (or use the precompiled version in extras/b_OTA.cpp.vxp) and copy it to OTA\b_OTA.cpp.vxp on the internal flash of the LinkIt ONE.

The Library will download the update.md5 and update.vxp file at the configured location and check if the md5sum within the update fila and the installed firmware differ. If the are not the same, the updater in OTA\b_OTA.vxp.cpp will be started to update the firmware.

## Example application using the update manager

    #include <OTAUpdate.h>
    #include <OTAUtils.h>
    
    #include <LStorage.h>
    #include <LFlash.h>
    #include <LGPRSClient.h>
    #include <LGPRS.h>

    void setup() {
      Serial.begin(9600);
    
      while (!Serial.available()) {
        delay(100);
        Serial.printf("\rpress any key to start...");
      }
      Serial.printf("\r\nstarted...\r\n");
      Serial.printf("init gprs... \r\n");
      while (!LGPRS.attachGPRS()) {
        delay(500);
      }
    
      OTAUpdate.begin("<host or ip>", "<path>");
    }
    
    void loop() {
      // put update.md5 and update.vxp files into OTA folder...
      while (true) {
        if (OTAUpdate.checkUpdate()) {
          OTAUpdate.startUpdate();
        }
    
        delay(10000);
      }
    }
    
## TODO
* use proper HTTP 1.1 download and parse HTTP headers
* implement RSA signature to verify integrity    
