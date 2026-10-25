#include <EasyMatter.h>
EasyMatterConfig config;
const uint8_t CONTACT_PIN=27;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Window contact"; config.deviceType=EasyMatterDeviceType::ContactSensor;
  EasyMatter.input(CONTACT_PIN, INPUT_PULLUP, 40); EasyMatter.begin(config);
}
void loop(){ EasyMatter.update(); }
