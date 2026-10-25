#include <EasyMatter.h>

EasyMatterConfig config;
void setup() {
  Serial.begin(115200);
  config.wifiSSID = "your-2.4GHz-ssid";
  config.wifiPassword = "your-password";
  config.vendor = "Your vendor";
  config.deviceName = "EasyMatter dimmer";
  config.deviceType = EasyMatterDeviceType::DimmableLight;
  EasyMatter.pwmOutput(4, 5000, 8);
  EasyMatter.onBrightnessChange([](uint8_t level) { Serial.printf("Brightness: %u\n", level); });
  EasyMatter.begin(config);
}
void loop() { EasyMatter.update(); }
