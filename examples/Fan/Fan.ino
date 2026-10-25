#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Ventilation fan"; config.deviceType=EasyMatterDeviceType::Fan;
  EasyMatter.pwmOutput(4, 20000, 8); EasyMatter.begin(config);
  EasyMatter.onBrightnessChange([](uint8_t duty){ Serial.printf("Fan output duty %u/255\n", duty); });
}
void loop(){ EasyMatter.update(); EasyMatter.setPwm(uint8_t(uint16_t(EasyMatter.fanSpeed())*255/100)); }
