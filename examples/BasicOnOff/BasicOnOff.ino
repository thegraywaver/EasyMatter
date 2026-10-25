#include <EasyMatter.h>

EasyMatterConfig config;

void setup() {
  Serial.begin(115200);
  config.wifiSSID = "your-2.4GHz-ssid";
  config.wifiPassword = "your-password";
  config.vendor = "Your registered vendor name";
  config.deviceName = "EasyMatter Light";
  config.debug = true;

  EasyMatter.output(2, OUTPUT);
  EasyMatter.button(0); // Replace with a suitable button GPIO for your board.
  if (!EasyMatter.begin(config)) Serial.println("EasyMatter initialization failed");
}

void loop() {
  EasyMatter.update();
  digitalWrite(2, EasyMatter.isOn() ? HIGH : LOW);
}
