#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="Room climate"; config.deviceType=EasyMatterDeviceType::TemperatureHumiditySensor;
  config.initialTemperatureC=21.5f; config.initialHumidityPercent=45.0f; EasyMatter.begin(config);
}
void loop(){
  EasyMatter.update();
  // Feed values from your chosen I2C/SPI sensor driver here.
  EasyMatter.sensor("temperature").setValue(21.5f);
  EasyMatter.sensor("humidity").setValue(45.0f);
  delay(5000);
}
