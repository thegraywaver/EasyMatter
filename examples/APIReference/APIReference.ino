#include <EasyMatter.h>

/*
  EasyMatter API reference
  ------------------------
  This sketch is a command/configuration catalog and a working dimmable-light
  example. Open Serial Monitor at 115200 baud and enter a command:
    on, off, toggle, brightness 0..255, status

  Configure one endpoint profile before begin(). One deviceType is active per
  Matter node (temperature+humidity creates both sensor endpoints).

  EasyMatterConfig options:
    wifiSSID, wifiPassword : 2.4 GHz Wi-Fi credentials
    vendor, deviceName     : Matter product labels
    deviceType             : one EasyMatterDeviceType profile below
    debug                  : reserved option (currently no extra debug output)
    initialState           : initial on/off state
    initialBrightness      : 0..255
    initialColorTemperatureMireds : initial tunable-white temperature
    initialTemperatureC, initialHumidityPercent : initial sensor values

  Profiles:
    EasyMatterDeviceType::Light, EasyMatterDeviceType::DimmableLight, EasyMatterDeviceType::RgbLight,
    EasyMatterDeviceType::ColorTemperatureLight, EasyMatterDeviceType::EnhancedColorLight,
    EasyMatterDeviceType::Plug, EasyMatterDeviceType::DimmablePlug,
    EasyMatterDeviceType::TemperatureSensor, EasyMatterDeviceType::HumiditySensor,
    EasyMatterDeviceType::TemperatureHumiditySensor, EasyMatterDeviceType::ContactSensor,
    EasyMatterDeviceType::MotionSensor, EasyMatterDeviceType::Thermostat, EasyMatterDeviceType::PressureSensor,
    EasyMatterDeviceType::IlluminanceSensor, EasyMatterDeviceType::WaterLeakSensor, EasyMatterDeviceType::WaterFreezeSensor,
    EasyMatterDeviceType::RainSensor, EasyMatterDeviceType::GenericSwitch, EasyMatterDeviceType::Fan,
    EasyMatterDeviceType::WindowCovering, EasyMatterDeviceType::TemperatureControlledCabinet.

  API reference (only use commands matching the selected profile):
    Lifecycle/status: begin(config), update(), isReady(), isCommissioned(),
      pairingCode(), qrCodeUrl(). Call update() frequently from loop().
    On/off: setOn(bool), isOn(), onChange(callback)
    Level/color: setBrightness(0..255), brightness(), color(r,g,b),
      setColorTemperature(mireds), colorTemperature(),
      onBrightnessChange(callback), onColorChange(callback),
      onColorTemperatureChange(callback)
    Sensors: setTemperature(C), temperature(), setHumidity(%), humidity(),
      setContact(closed), isContactClosed(), setMotion(occupied),
      isMotionDetected(), setPressure(hPa), pressure(),
      setIlluminance(lux), illuminance(), setLeak(bool), isLeakDetected(),
      setFreeze(bool), isFreezeDetected(), setRain(bool), isRaining().
      sensor("temperature" or "humidity") provides setValue(), setRange(),
      valid(), value(), updatedAt(). Hardware sensor drivers are separate.
    Thermostat: setLocalTemperature(C), setHeatingSetpoint(C),
      setCoolingSetpoint(C), temperature(), heatingSetpoint(),
      coolingSetpoint(). Connect these to your own safe HVAC control logic.
    Generic switch: switchClick(); button(pin, debounceMs) can emit local
      button events when the selected profile supports them.
    Fan: setFanSpeed(0..100), fanSpeed(). setOn() controls fan on/off.
    Window: setLiftPercentage(0..100), liftPercentage(),
      onLiftTarget(callback). Drive the motor yourself and report its position.
    Cabinet: setCabinetSetpoint(C), cabinetSetpoint().
    GPIO: output(pin, mode, activeHigh), pwmOutput(pin, Hz, resolutionBits),
      setPwm(0..255), input(pin, mode, debounceMs), button(pin, debounceMs),
      analogInput(pin, resolutionBits), analogValue().
    Optional adapters: display.attach(begin,title,line,show), then
      display.begin/title/line/show; ir.attach(begin,send,context), then
      ir.begin()/send(protocol,code,bits). These do not add Matter endpoints.
    Resets: resetPairing(), resetSettings(), resetWiFi(), factoryReset().
      These erase state; factoryReset clears Matter pairing and Wi-Fi too.
*/

EasyMatterConfig config;
String command;

void onStateChanged(bool state) {
  Serial.printf("On/off changed: %s\n", state ? "ON" : "OFF");
}

void printStatus() {
  Serial.printf("Ready=%s commissioned=%s on=%s brightness=%u\n",
    EasyMatter.isReady() ? "yes" : "no",
    EasyMatter.isCommissioned() ? "yes" : "no",
    EasyMatter.isOn() ? "yes" : "no",
    EasyMatter.brightness());
  if (EasyMatter.isReady()) {
    Serial.printf("Pairing code: %s\nQR URL: %s\n",
      EasyMatter.pairingCode(), EasyMatter.qrCodeUrl());
  }
}

void setup() {
  Serial.begin(115200);
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.vendor = "Your vendor";
  config.deviceName = "EasyMatter reference light";
  config.deviceType = EasyMatterDeviceType::DimmableLight;
  config.debug = false;
  config.initialState = false;
  config.initialBrightness = 128;
  config.initialColorTemperatureMireds = 370;
  config.initialTemperatureC = 20.0f;
  config.initialHumidityPercent = 50.0f;

  EasyMatter.onChange(onStateChanged);
  EasyMatter.output(2); // Optional low-voltage LED/driver output; choose a safe pin.
  if (!EasyMatter.begin(config)) {
    Serial.println("EasyMatter.begin failed; check profile and Wi-Fi configuration.");
    return;
  }
  Serial.println("Commands: on | off | toggle | brightness 0..255 | status");
}

void loop() {
  EasyMatter.update();
  if (!Serial.available()) return;
  command = Serial.readStringUntil('\n');
  command.trim();
  if (command == "on") EasyMatter.setOn(true);
  else if (command == "off") EasyMatter.setOn(false);
  else if (command == "toggle") EasyMatter.setOn(!EasyMatter.isOn());
  else if (command == "status") printStatus();
  else if (command.startsWith("brightness ")) {
    int value = command.substring(11).toInt();
    if (value < 0 || value > 255 || !EasyMatter.setBrightness((uint8_t)value))
      Serial.println("Brightness unsupported or out of range (0..255).");
  } else Serial.println("Unknown command. Use on, off, toggle, brightness 0..255, status.");
}
