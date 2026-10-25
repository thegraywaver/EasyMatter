#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Room thermostat"; config.deviceType=EasyMatterDeviceType::Thermostat;
  config.initialTemperatureC=20; EasyMatter.begin(config);
  EasyMatter.setHeatingSetpoint(19); EasyMatter.setCoolingSetpoint(24);
}
void loop(){
  EasyMatter.update();
  // Replace with the real measured room temperature and HVAC control logic.
  EasyMatter.setLocalTemperature(20.0f);
  delay(5000);
}
