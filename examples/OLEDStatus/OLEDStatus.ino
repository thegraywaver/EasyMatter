#include <EasyMatter.h>
EasyMatterConfig config;
void setup(){
  Serial.begin(115200); config.wifiSSID="your-ssid"; config.wifiPassword="your-password";
  config.vendor="Your vendor"; config.deviceName="EasyMatter display demo";
  EasyMatter.begin(config);
  // Attach wrappers around your optional SSD1306/SH1106 library.
  EasyMatter.display.attach([](){}, [](const char*){}, [](uint8_t,const char*){}, [](){});
  EasyMatter.display.begin(); EasyMatter.display.title("EasyMatter");
  EasyMatter.display.line(0,"Matter started"); EasyMatter.display.show();
}
void loop(){ EasyMatter.update(); }
