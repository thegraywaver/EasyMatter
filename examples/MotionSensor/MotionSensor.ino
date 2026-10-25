#include <EasyMatter.h>
EasyMatterConfig config;
const uint8_t PIR_PIN=27;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Motion sensor"; config.deviceType=EasyMatterDeviceType::MotionSensor;
  EasyMatter.input(PIR_PIN, INPUT, 30); EasyMatter.begin(config);
}
void loop(){ EasyMatter.update(); }
