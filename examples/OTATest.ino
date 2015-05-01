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
  while (!LGPRS.attachGPRS("Free", NULL, NULL)) {
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
