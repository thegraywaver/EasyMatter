#include <EasyMatter.h>

EasyMatterConfig config;

void setup() {
  Serial.begin(115200);
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.vendor = "Your vendor";
  config.deviceName = "Doorbell button";
  config.deviceType = EasyMatterDeviceType::Doorbell;

  EasyMatter.button(18);
  EasyMatter.pairingResetButton(4);

  if (!EasyMatter.begin(config)) {
    Serial.println("EasyMatter initialization failed");
  }
}

void loop() {
  EasyMatter.update();
}
