#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Cooler cabinet";
  config.deviceType=EasyMatterDeviceType::TemperatureControlledCabinet; config.initialTemperatureC=4.0f;
  EasyMatter.begin(config);
}
void loop(){
  EasyMatter.update();
  float targetC=EasyMatter.cabinetSetpoint();
  // Read actual cabinet temperature and control the refrigeration system safely.
  (void)targetC;
  delay(500);
}
