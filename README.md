# OTAUpdate
OTAUpdate is a Library to perform Over-The-Air Updates of your [LinkIt ONE](https://labs.mediatek.com/site/global/developer_tools/mediatek_linkit/whatis_linkit/index.gsp). The library can be used to check for firmware updates and install them remotely into the flash of the board. 


The update check and download is performed using HTTP connections with the included GPRS Module.

Please follow these steps :

- Open arduino IDE. In File->Preference, check option for Verbose compilation. It is necessary to display where are precompiled files (temporary files)

- Load and build examples/OTA.ino sketch. Thanks to verbose info, you can copy, paste and rename precompiled file OTA.cpp.vxp into internal flash of the LinkIt ONE : OTA\b_OTA.cpp.vxp 

- Load and build your final sketche. Get precompiled file and rename it update.vxp

- Compute md5 file. On a Linux machine run this command
	$md5sum update.vxp > update.md5sum
	
- Copy update.vxp and update.md5 on server folder (the one you specify IP adress and path in your sketche)


The Library will download the update.md5 and update.vxp file at the configured location and check if the md5sum within the update file and the installed firmware differ. If the are not the same, the updater in OTA\b_OTA.vxp.cpp will be started to update the firmware.

## Example application using the update manager

    #include <OTAUpdate.h>
    #include <OTAUtils.h>
    
    #include <LStorage.h>
    #include <LFlash.h>
    #include <LGPRSClient.h>
    #include <LGPRS.h>

    void setup() {
      Serial.begin(115200);
    
      while (!Serial.available()) {
        delay(100);
        Serial.printf("\rpress any key to start...");
      }
      Serial.printf("\r\nstarted...\r\n");
      Serial.printf("init gprs... \r\n");
      while (!LGPRS.attachGPRS()) {
        delay(500);
      }
    
      OTAUpdate.begin("<host or ip>", "<port>", "<path>");
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
	
## Debug info of a working update

	press any key to start...
	started...
	init gprs...
	OTAUpdate::begin - my.ip.adress.server OTA
	OTAUpdateClass::begin - reading autostart.txt
	autostart.txt: App=C:\MRE\ineedupdate.cpp.vxp
	vm_md5sum - checking file C:\MRE\ineedupdate.cpp.vxp
	vm_md5sum - C:\MRE\ineedupdate.cpp.vxp: 8a54ea1718d130f2057a3291f62cc98f
	OTAUpdate::begin - C:\MRE\ineedupdate.cpp.vxp [8a54ea1718d130f2057a3291f62cc98f]
	OTAUpdate::downloadFile my.ip.adress.server:80 'GET /OTA/update.md5'
	size = 286
	OTAUpdate::downloadFile - done! got 286 bytes
	OTAUpdate::DeleteHTTPHeader - end of HTTP header reached
	OTAUpdate::DeleteHTTPHeader - OTA\update.md5 file replace with no HTTP header
	OTAUpdate::parseUpdateMD5 - update.vxp [1fc27514eb1d9712b9c09f2c5f5a0776]
	OTAUpdate::checkMD5 - C:\MRE\ineedupdate.cpp.vxp 1fc27514eb1d9712b9c09f2c5f5a0776
	vm_md5sum - checking file C:\MRE\ineedupdate.cpp.vxp
	vm_md5sum - C:\MRE\ineedupdate.cpp.vxp: 8a54ea1718d130f2057a3291f62cc98f
	OTAUpdate::checkMD5 - error md5sum mismatch!
	found a new firmware update.vxp [1fc27514eb1d9712b9c09f2c5f5a0776]!
	OTAUpdate::downloadFile my.ip.adress.server:80 'GET /OTA/update.vxp'
	size = 64199
	OTAUpdate::downloadFile - done! got 64199 bytes
	OTAUpdate::DeleteHTTPHeader - end of HTTP header reached
	OTAUpdate::DeleteHTTPHeader - OTA\update.vxp file replace with no HTTP header
	OTAUpdate::checkMD5 - C:\OTA\update.vxp 1fc27514eb1d9712b9c09f2c5f5a0776
	vm_md5sum - checking file C:\OTA\update.vxp
	vm_md5sum - C:\OTA\update.vxp: 1fc27514eb1d9712b9c09f2c5f5a0776
	OTAUpdate::checkMD5 - OK
	new firmware is ok!
	OTAUpdate::parseUpdateMD5 - update.vxp [1fc27514eb1d9712b9c09f2c5f5a0776]
	OTAUpdate::checkMD5 - C:\OTA\update.vxp 1fc27514eb1d9712b9c09f2c5f5a0776
	vm_md5sum - checking file C:\OTA\update.vxp
	vm_md5sum - C:\OTA\update.vxp: 1fc27514eb1d9712b9c09f2c5f5a0776
	OTAUpdate::checkMD5 - OK
	OTAUpdate::startUpdate - updating to firmware update.vxp [1fc27514eb1d9712b9c09f2c5f5a0776]
	OTAUpdate::startFirmware - C:\OTA\b_OTA.cpp.vxp
	calling vm_reboot_normal_start()

AFTER Rebooting

	OTAUpdate::begin -
	OTAUpdateClass::begin - reading autostart.txt
	autostart.txt: App=C:\OTA\b_OTA.cpp.vxp
	vm_md5sum - checking file C:\OTA\b_OTA.cpp.vxp
	vm_md5sum - C:\OTA\b_OTA.cpp.vxp: 8efffeca19b39def1f992ffdb18e170c
	OTAUpdate::begin - C:\OTA\b_OTA.cpp.vxp [8efffeca19b39def1f992ffdb18e170c]
	OTAUpdate::parseUpdateMD5 - update.vxp [1fc27514eb1d9712b9c09f2c5f5a0776]
	OTAUpdate::checkMD5 - C:\OTA\update.vxp 1fc27514eb1d9712b9c09f2c5f5a0776
	vm_md5sum - checking file C:\OTA\update.vxp
	vm_md5sum - C:\OTA\update.vxp: 1fc27514eb1d9712b9c09f2c5f5a0776
	OTAUpdate::checkMD5 - OK
	OTAUpdate::startFirmware - C:\MRE\update.vxp
	calling vm_reboot_normal_start()

RUNNING THE UPDATED SKETCHE (datetime.ino example)
	
	rtc = 1072933422
	datetimeInfo t.day/t.mon/t.year  t.hour:t.min:t.sec =    1/ 1/2004   5: 3:42
	rtc = 1072933423
	datetimeInfo t.day/t.mon/t.year  t.hour:t.min:t.sec =    1/ 1/2004   5: 3:43
	rtc = 1072933424
	datetimeInfo t.day/t.mon/t.year  t.hour:t.min:t.sec =    1/ 1/2004   5: 3:44
