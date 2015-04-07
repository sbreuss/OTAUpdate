#ifndef OTAUTILS_H
#define OTAUTILS_H


#define DO_DEBUG_UTILS

#ifndef DO_DEBUG_UTILS
#define DEBUG_UTILS(...)
#else
#define DEBUG_UTILS(...) Serial.printf(__VA_ARGS__)
#endif

#define OTA_MAX_PATH_LEN   		1024

#define DIGEST_SIZE_MD5			16
#define DIGEST_SIZE_SHA256		32
#define DIGEST_SIZE_MAX			32
#define DIGEST_SIZE_CHAR		128
#define DIGEST_SIZE_BUFFER		1024


#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

boolean vm_md5sum(const char* name, char* digest);
boolean md5sum(const char* name, char* digest);

boolean vm_sha256sum(const char* name, char* digest);
boolean sha256sum(const char* name, char* digest);

void vm_reset(void);
void reset(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef OTAUTILS_H