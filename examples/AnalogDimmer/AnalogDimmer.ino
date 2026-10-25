#include <EasyMatter.h>

EasyMatterConfig config;

const uint8_t potentiometerPin = 34;
const uint32_t readIntervalMs = 25;
const uint8_t brightnessDeadband = 2;

uint32_t lastReadAt = 0;
uint8_t lastAppliedBrightness = 0;
bool hasAppliedBrightness = false;

void setup() {
  Serial.begin(115200);

  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.vendor = "Your vendor";
  config.deviceName = "Analog dimmable light";
  config.deviceType = EasyMatterDeviceType::DimmableLight;

  EasyMatter.pwmOutput(4, 5000, 8);
  EasyMatter.analogInput(potentiometerPin, 12);

  if (!EasyMatter.begin(config)) {
    Serial.println("EasyMatter initialization failed");
  }
}

void loop() {
  EasyMatter.update();

  const uint32_t now = millis();
  if (now - lastReadAt < readIntervalMs) {
    return;
  }
  lastReadAt = now;

  const uint16_t analogValue = EasyMatter.analogValue();
  const uint8_t brightness = static_cast<uint8_t>((uint32_t(analogValue) * 255) / 4095);
  const int difference = int(brightness) - int(lastAppliedBrightness);

  if (!hasAppliedBrightness || abs(difference) >= brightnessDeadband) {
    if (brightness == 0) {
      EasyMatter.setOn(false);
    } else {
      EasyMatter.setBrightness(brightness);
      EasyMatter.setOn(true);
    }

    lastAppliedBrightness = brightness;
    hasAppliedBrightness = true;
  }
}
