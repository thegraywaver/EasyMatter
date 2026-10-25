#include <EasyMatter.h>
EasyMatterConfig config;
void setup() {
  Serial.begin(115200);
  config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="EasyMatter RGB"; config.deviceType=EasyMatterDeviceType::RgbLight;
  EasyMatter.begin(config);
  EasyMatter.onColorChange([](uint8_t r,uint8_t g,uint8_t b){ Serial.printf("RGB %u,%u,%u\n",r,g,b); });
  EasyMatter.color(255,80,20);
}
void loop(){ EasyMatter.update(); }
