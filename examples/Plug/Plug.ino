#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="EasyMatter plug"; config.deviceType=EasyMatterDeviceType::Plug;
  // Use a suitable isolated relay module. Never connect mains to a GPIO.
  EasyMatter.output(26, OUTPUT); EasyMatter.begin(config);
}
void loop(){ EasyMatter.update(); digitalWrite(26, EasyMatter.isOn()?HIGH:LOW); }
