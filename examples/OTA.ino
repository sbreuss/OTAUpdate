
#include <OTAUpdate.h>
#include <OTAUtils.h>

#include <LStorage.h>
#include <LFlash.h>
#include <LGPRSClient.h>



void setup() {
  Serial.begin(9600);
  OTAUpdate.begin();
}

void loop() {
  while(!OTAUpdate.performUpdate()) {
    // the update failed... what to do now?
    Serial.printf("OTA update failed! Please update manually... :(\r\n");
    delay(1000);
  }
}
