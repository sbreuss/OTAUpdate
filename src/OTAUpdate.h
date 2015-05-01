#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#define DO_DEBUG_UPDATE

#ifndef DO_DEBUG_UPDATE
#define DEBUG_UPDATE(...)
#else
#define DEBUG_UPDATE(...) Serial.printf(__VA_ARGS__)
#endif

#define OTA_FW		"OTA\\b_OTA.cpp.vxp"
#define UPDATE_VXP	"OTA\\update.vxp"
#define UPDATE_MD5	"OTA\\update.md5"
#define TMPFILE		"OTA\\tmp.txt"


#include <Arduino.h>
#include <LTask.h>

#include <LStorage.h>
#include <LFlash.h>
#include <LGPRSClient.h>

#include "OTAUtils.h"

class OTAUpdateClass {
	
public:
	OTAUpdateClass(void);
	boolean getVersionInfo(void);
	
	// DESCRIPTION
	//  Initialize the OTA updater with host and path
	// RETURNS
	//  true if succeed, false if failed
	boolean begin(const char* host="", const char* path="");
	
	boolean getFirmwareName(char* name, size_t len);
	boolean getFirmwareDigest(char* digest, size_t len);
	
	boolean checkUpdate(void);
	boolean startUpdate(void);
	boolean performUpdate(void);
	
private:
	boolean initialized;
	char firmware_digest[DIGEST_SIZE_CHAR];
	char firmware_name[OTA_MAX_PATH_LEN];
	char host[OTA_MAX_PATH_LEN];
	char path[OTA_MAX_PATH_LEN];
	
	boolean downloadFile(const char* name);
	boolean parseUpdateMD5(String* vxp_name, String* vxp_digest);
	boolean checkMD5(const char* name, const char* digest);
	boolean checkUpdateFiles(String* vxp_update, String* vxp_digest);
	boolean startFirmware(const char* name);
	boolean copyFile(const char* src, const char* dst);
	boolean DeleteHTTPHeader(const char* name);
};

extern OTAUpdateClass OTAUpdate;

#endif // #ifndef OTAUPDATE_H