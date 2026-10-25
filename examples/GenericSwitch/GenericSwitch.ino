#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="EasyMatter button"; config.deviceType=EasyMatterDeviceType::GenericSwitch;
  EasyMatter.button(27); EasyMatter.begin(config);
}
void loop(){ EasyMatter.update(); }
