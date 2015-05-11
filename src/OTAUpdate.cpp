#include "OTAUpdate.h"
#include "OTAUtils.h"



OTAUpdateClass::OTAUpdateClass() {
	this->initialized = false;
}

boolean OTAUpdateClass::begin(const char* host, const char* port, const char* path) {
	DEBUG_UPDATE("OTAUpdate::begin - %s %s\r\n", host, path);
	
	// initialize our memory structures
	this->initialized = false;
	memset(this->firmware_name, 0, OTA_MAX_PATH_LEN);
	memset(this->firmware_digest, 0, DIGEST_SIZE_CHAR);
	memset(this->host, 0, OTA_MAX_PATH_LEN);
	memset(this->path, 0, OTA_MAX_PATH_LEN);
	memset(this->port, 0, OTA_MAX_PATH_LEN);
	strncpy(this->host, host, OTA_MAX_PATH_LEN-1);
	strncpy(this->path, path, OTA_MAX_PATH_LEN-1);
	strncpy(this->port, port, OTA_MAX_PATH_LEN-1);
	
	// read the firmware information
	LFlash.begin();
	LFile cfg = LFlash.open("autostart.txt", FILE_READ);
    if (!cfg) {
		DEBUG_UPDATE("OTAUpdateClass::begin - could not read autostart.txt\r\n");
		return false;
	}
	
	DEBUG_UPDATE("OTAUpdateClass::begin - reading autostart.txt\r\n");
	while (cfg.available()) {
		String line = "";
		char c = '\n';
		// read the setting part of the line
		while (cfg.available()) {
			c = cfg.read();
			line += c;
			if(c == '\n') {
				break;
			}
		}

		// look for = in the config line
		line.trim();
		int idx = line.indexOf("=");
		
		if(idx >= 0) {
			String setting = line.substring(0, idx);
			String value = line.substring(idx+1);
			
			setting.trim();
			value.trim();
			
			DEBUG_UPDATE("autostart.txt: %s=%s\r\n", setting.c_str(), value.c_str());
			if(setting == "App") {
				value.toCharArray(firmware_name, OTA_MAX_PATH_LEN);
				this->initialized = true;
				break;
			}
		}
	}
	cfg.close();
	
	if(this->initialized) {
		// we found the app name... calculate the md5 sum
		md5sum(this->firmware_name, this->firmware_digest);
		DEBUG_UPDATE("OTAUpdate::begin - %s [%s]\r\n", this->firmware_name, this->firmware_digest);
	} else {
		DEBUG_UPDATE("OTAUpdate::begin - could not find firmware name\r\n");
		return false;
	}
	
	return true;
}


boolean OTAUpdateClass::getFirmwareName(char* name, size_t size) {
	if(this->initialized) {
		strncpy(name, this->firmware_name, size);
		return true;
	} else {
		name[0] = 0;
		return false;
	}
}

boolean OTAUpdateClass::getFirmwareDigest(char* digest, size_t size) {
	if(this->initialized) {
		strncpy(digest, this->firmware_digest, size);
		return true;
	} else {
		digest[0] = 0;
		return false;
	}
}

boolean OTAUpdateClass::checkUpdate(void) {
	String vxp_name, vxp_digest;
	
	if(!downloadFile(UPDATE_MD5)) {
		return false;
	}
		
	if(!parseUpdateMD5(&vxp_name, &vxp_digest)) {
		return false;
	}
	
	if(checkMD5(this->firmware_name, vxp_digest.c_str())) {
		DEBUG_UPDATE("found no new firmware!\r\n");
		return false;
	}
	
	DEBUG_UPDATE("found a new firmware %s [%s]!\r\n", vxp_name.c_str(), vxp_digest.c_str());
	if(!downloadFile(UPDATE_VXP)) {
		return false;
	}
	
	if(!checkMD5("C:\\" UPDATE_VXP, vxp_digest.c_str())) {
		DEBUG_UPDATE("new firmware has a wrong md5sum!\r\n");
		return false;
	}
	
	DEBUG_UPDATE("new firmware is ok!\r\n");
	return true;
}


boolean OTAUpdateClass::startUpdate(void) {
	String vxp_name, vxp_digest;
	
	// check if all required files exist
	if(!LFlash.exists((char*)OTA_FW) || !LFlash.exists((char*)UPDATE_MD5) || !LFlash.exists((char*)UPDATE_VXP)) {
		DEBUG_UPDATE("OTAUpdate::startUpdate - not all required files found\r\n");
		return false;
	}
	
	// check if the update files are ok
	if(!checkUpdateFiles(&vxp_name, &vxp_digest)) {
		DEBUG_UPDATE("OTAUpdate::startUpdate - check files failed\r\n");
		return false;
	}
	
	// if yes, start the OTA FW upgrade
	DEBUG_UPDATE("OTAUpdate::startUpdate - updating to firmware %s [%s]\r\n", vxp_name.c_str(), vxp_digest.c_str());
	return startFirmware("C:\\" OTA_FW);	
}



boolean OTAUpdateClass::performUpdate(void) {
	String vxp_name, vxp_digest;
	
	// check if our update file is ok
	if(!checkUpdateFiles(&vxp_name, &vxp_digest)) {
		return false;
	}
	
	// copy it into its new place
	if(!copyFile(UPDATE_VXP, ("MRE\\" + vxp_name).c_str())) {
		return false;
	}
	
	// remove the old files
	LFlash.begin();
	LFlash.remove((char*)UPDATE_MD5);
	LFlash.remove((char*)UPDATE_VXP);
	
	// update autostart.txt to start the new firmware
	startFirmware(("C:\\MRE\\" + vxp_name).c_str());
	return true;
}

boolean OTAUpdateClass::downloadFile(const char* name) {
	// make some http requests to check for firmware updates
	LGPRSClient c;
	uint8_t buffer[DIGEST_SIZE_BUFFER];
	int n , size, max_millis;
	char buff[256];
	static char endofheader[5] ;
	boolean HTTPHeaderreached = false;
	char byc;
	
	//convert string to int
	String sthostport = this->port;
	unsigned int uinthostport = sthostport.toInt();
	
	// download the firmware
	if(!c.connect(this->host, uinthostport)) {
		DEBUG_UPDATE("OTAUpdate::downloadFile - error connecting to update host\r\n");
		return false;
	}
	// connected... send the get request
	DEBUG_UPDATE("OTAUpdate::downloadFile %s:%d 'GET /%s/%s'\r\n", this->host, uinthostport, this->path, &name[4]);
	
	sprintf(buff, "GET /%s/%s", this->path, &name[4]);
	c.print(buff);
	//c.printf("GET /%s/%s", this->path, &name[4]);
	c.println(" HTTP/1.1");
    c.print("Host: ");
    c.println(this->host);
    c.println("Connection: close");
    c.println();

	// save the result
	max_millis = millis() + 10000;
	LFlash.begin();
	LFlash.remove((char*) name);
	
	LFile ota = LFlash.open(name, FILE_WRITE);
	ota.seek(0);
	size = 0;
	
	// get data content of the file
	while(c.connected()) {
		//skip byte until end of HTTP Header
		if(HTTPHeaderreached == false){
			// read byte
			byc = c.read();
			if(byc > 0) {
				max_millis = millis() + 2000;
				// if HTTP header is not reached, read until find double CRLF
				Serial.print(byc);
				
				// proceed a right shift of the  array
				for(int i = 0; i < 3; i++){
					endofheader[i] =  endofheader[i+1];
				}
				// add last received char at the end of the array
				endofheader[3] = byc;
				//don't forget null caracter
				endofheader[4] = '\0';	
				// compare array with end of HTTP header key (double CRLF)
				if (strcmp("\r\n\r\n", endofheader ) == 0){
					// return true
					DEBUG_UPDATE("OTAUpdate::downloadFile - end of HTTP header reached\r\n");
					HTTPHeaderreached = true;
				}
				else{
					HTTPHeaderreached = false;
				}
			}
			else {
				if(millis() > max_millis) {
					DEBUG_UPDATE("OTAUpdate::downloadFile - timed out!\r\n");
					c.stop();
					ota.close();
					return false;
				} else {
					delay(100);
				}
			}							
		}
		else{
			int n = c.read(buffer, 1024);
			if(n > 0) {
				max_millis = millis() + 2000;
				ota.write(buffer, n);
				size += n;
				DEBUG_UPDATE("size = %d\r", size);
			} else {
				if(millis() > max_millis) {
					DEBUG_UPDATE("OTAUpdate::downloadFile - timed out!\r\n");
					c.stop();
					ota.close();
					return false;
				} else {
					delay(100);
				}
			}
		}
	}
	c.stop();
	ota.close();
	
	DEBUG_UPDATE("\r\nOTAUpdate::downloadFile - done! got %d bytes\r\n", size);
	return size > 0;
}

boolean OTAUpdateClass::parseUpdateMD5(String* vxp_name, String* vxp_digest) {
	LFlash.begin();
	LFile update_md5 = LFlash.open(UPDATE_MD5);
	if(!update_md5) {
		DEBUG_UPDATE("OTAUpdate::parseUpdateMD5 - could not open %s\r\n", UPDATE_MD5);
		return false;
	}

	String line = "";
	while(update_md5.available()) {		
		line += (char) update_md5.read();
	}
	update_md5.close();
	line.trim();
	
	// Warning : buffer line contain HTTP header !
	// should look for vxp name and md5 from the end of buffer
	
	// look for index of last word (md5 name file)
	int idx = line.lastIndexOf(" ");
	// get md4 name file from index until end of buffer
	*vxp_name = line.substring(idx);
	// get md5 from index and index - 32 bytes (lenght of md5 is constant)
	*vxp_digest = line.substring(idx - 33, idx - 1);
	
	vxp_name->trim();
	vxp_digest->trim();
	
	DEBUG_UPDATE("OTAUpdate::parseUpdateMD5 - %s [%s]\r\n", vxp_name->c_str(), vxp_digest->c_str());
	return true;
}

boolean OTAUpdateClass::checkMD5(const char* name, const char* hash) {
	char local_hash[DIGEST_SIZE_CHAR];	
	DEBUG_UPDATE("OTAUpdate::checkMD5 - %s %s\r\n", name, hash);
	
	// calculate the md5 sum of the new firmware
	if(!md5sum(name, local_hash)) {
		DEBUG_UPDATE("OTAUpdate::checkMD5 - error calculating md5sum!\r\n");
		return false;
	}
	
	// check if the md5sum of the firmware matches
	if(strcmp(hash, local_hash) != 0) {
		DEBUG_UPDATE("OTAUpdate::checkMD5 - error md5sum mismatch!\r\n");
		return false;
	}
	
	DEBUG_UPDATE("OTAUpdate::checkMD5 - OK\r\n");
	return true;
}

boolean OTAUpdateClass::checkUpdateFiles(String* vxp_name, String* vxp_digest) {
	// parse updateMD5
	if(!parseUpdateMD5(vxp_name, vxp_digest)) {
		return false;
	}
	
	// check the md5sum of the new vxp file
	if(!checkMD5("C:\\" UPDATE_VXP, vxp_digest->c_str())) {		
		return false;
	}
	
	return true;
}

boolean OTAUpdateClass::startFirmware(const char* name) {
	DEBUG_UPDATE("OTAUpdate::startFirmware - %s\r\n", name);
	
	// update autostart.txt for the new firmware
	LFlash.begin();
	LFile dst = LFlash.open("autostart.txt", FILE_WRITE);
	if(!dst) {
		DEBUG_UPDATE("OTAUpdate::performUpdate - error opening autostart.txt\r\n");
		return false;
	}
	
	dst.seek(0);
	dst.printf("[autostart]\r\nApp=%s\r\n", name);
	dst.close();
	
	// reset the board
	reset();
	return true;
}

boolean OTAUpdateClass::copyFile(const char* src, const char* dst) {
	char buffer[DIGEST_SIZE_BUFFER];
	
	LFlash.begin();
	LFile fsrc = LFlash.open(src, FILE_READ);
	if(!fsrc) {
		DEBUG_UPDATE("OTAUpdate::copyFile - error opening src %s\r\n", src);
		return false;
	}
	LFile fdst = LFlash.open(dst, FILE_WRITE);
	if(!fsrc) {
		fsrc.close();
		DEBUG_UPDATE("OTAUpdate::copyFile - error opening dst %s\r\n", dst);
		return false;
	}
	
	fdst.seek(0);
	int size = fsrc.size();
	int done = 0;
	while(done < size) {
		int read = fsrc.read(buffer, DIGEST_SIZE_BUFFER);
		fdst.write(buffer, read);
		done += read;		
	}
	fsrc.close();
	fdst.close();
	
	return true;
}



OTAUpdateClass OTAUpdate;