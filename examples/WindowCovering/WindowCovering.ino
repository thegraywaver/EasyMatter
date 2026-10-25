#include <EasyMatter.h>
EasyMatterConfig config;
void targetPosition(uint8_t percent){ Serial.printf("Matter target position: %u%% closed\n",percent); /* drive your blind motor here */ }
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Window blind"; config.deviceType=EasyMatterDeviceType::WindowCovering;
  EasyMatter.onLiftTarget(targetPosition); EasyMatter.begin(config);
}
void loop(){ EasyMatter.update(); /* report measured motor position with setLiftPercentage() */ }
