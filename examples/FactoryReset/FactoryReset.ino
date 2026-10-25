#include <EasyMatter.h>

EasyMatterConfig config;
void setup() {
  Serial.begin(115200);
  config.wifiSSID = "your-2.4GHz-ssid";
  config.wifiPassword = "your-password";
  config.vendor = "Your vendor";
  config.deviceName = "EasyMatter reset demo";
  EasyMatter.begin(config);
  Serial.println("Send R in Serial Monitor to remove Matter fabric");
}
void loop() {
  EasyMatter.update();
  if (Serial.available() && Serial.read() == 'R') {
    Serial.println(EasyMatter.resetPairing() ? "Pairing reset requested" : "Reset failed");
  }
}
