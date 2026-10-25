# EasyMatter

EasyMatter is an Arduino C++11 wrapper for Espressif's official Arduino Matter endpoints. It does not implement Matter or invent endpoint types. The current code supports the original ESP32-WROOM-32 and the Espressif Arduino-ESP32 Matter build; the requested Arduino-ESP32 3.3.12 target has not been compiled in this environment.

#VERIFIED CHIPS

| DEVICE | TESTED | WORKING |
|---|---|---|
| ESP-WROOM32 | ✅ | ✅ |
| Arduino Nano ESP32 | ✅ | ✅ |
| Arduino Uno R4 WIFI | ✅ | ❌ |
| ESP32 CAM | ❌ | - |
| Any other ESP32 for now | ❌ | - |

## Simple on/off light

This example creates a basic Matter light. A button on GPIO 18 toggles the light, and a low-voltage LED or driver connected to GPIO 4 follows the Matter state. Connect the button between GPIO 18 and GND.

```cpp
#include <EasyMatter.h>

EasyMatterConfig config;

void setup() {
  Serial.begin(115200);
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.vendor = "Your vendor";
  config.deviceName = "Simple LED";
  config.deviceType = EasyMatterDeviceType::Light;

  EasyMatter.output(4, OUTPUT);
  EasyMatter.button(18);

  if (!EasyMatter.begin(config)) {
    Serial.println("EasyMatter initialization failed");
  }
}

void loop() {
  EasyMatter.update();
}
```

`EasyMatter.update()` reads the debounced button and keeps the GPIO output synchronized with the Matter on/off state. Use an LED with a suitable resistor or a low-voltage driver; never connect mains voltage directly to an ESP32 GPIO.

## Install and commissioning

Copy this folder into `Documents/Arduino/libraries/EasyMatter`, restart the IDE, select an original ESP32-WROOM-32 board, and select **Huge APP (3 MB No OTA / 1 MB SPIFFS)**. Espressif's Matter build needs a large app partition. On the original ESP32 prebuilt Matter library, use on-network Wi-Fi commissioning. `begin()` starts Wi-Fi from supplied credentials, constructs the endpoint, starts Matter, and prints the real pairing code and onboarding QR URL returned by Espressif. A QR URL is not an ASCII QR code; scan/use the code through the controller's supported onboarding flow. The controller must still commission the device onto its Matter fabric. For Alexa, start device addition in the Alexa app and scan the onboarding QR code or enter the manual code printed by the sketch; the exact app steps depend on Alexa app version and account/device support.

```cpp
#include <EasyMatter.h>
EasyMatterConfig config;
void setup() {
  Serial.begin(115200);
  config.wifiSSID = "your-2.4GHz-ssid";
  config.wifiPassword = "your-password";
  config.vendor = "Your vendor name";
  config.deviceName = "Kitchen light";
  config.deviceType = EasyMatterDeviceType::DimmableLight;
  EasyMatter.pwmOutput(4);
  EasyMatter.begin(config);
}
void loop() {
  EasyMatter.update();
  if (EasyMatter.isOn()) { /* local state is kept in sync */ }
}
```

Do not commit credentials. Choose a legal/owned vendor name and production commissioning credentials appropriate for a real product. The wrapper does not set vendor IDs, DACs or setup PINs.

## Endpoint profiles

Set `config.deviceType` before `EasyMatter.begin(config)`. Each profile creates Espressif's corresponding endpoint; temperature+humidity creates two endpoints on the same Matter node. Endpoints must be created before Matter starts, which `begin()` handles.

- Lights: `EasyMatterDeviceType::Light`, `EasyMatterDeviceType::DimmableLight`, `EasyMatterDeviceType::RgbLight`, `EasyMatterDeviceType::ColorTemperatureLight`, `EasyMatterDeviceType::EnhancedColorLight`.
- Outlets: `EasyMatterDeviceType::Plug`, `EasyMatterDeviceType::DimmablePlug`.
- Sensors: `EasyMatterDeviceType::TemperatureSensor`, `EasyMatterDeviceType::HumiditySensor`, `EasyMatterDeviceType::TemperatureHumiditySensor`, `EasyMatterDeviceType::ContactSensor`, `EasyMatterDeviceType::MotionSensor`, `EasyMatterDeviceType::PressureSensor`, `EasyMatterDeviceType::IlluminanceSensor`, `EasyMatterDeviceType::WaterLeakSensor`, `EasyMatterDeviceType::WaterFreezeSensor`, `EasyMatterDeviceType::RainSensor`.
- Controls: `EasyMatterDeviceType::Thermostat`, `EasyMatterDeviceType::Fan`, `EasyMatterDeviceType::GenericSwitch`, `EasyMatterDeviceType::WindowCovering`, `EasyMatterDeviceType::TemperatureControlledCabinet`.

### Common API

- `setOn(bool)`, `isOn()`, `onChange(callback)` synchronize supported on/off endpoints. Matter controller changes are polled by `update()` and become visible to local code.
- `setBrightness(uint8_t)` / `brightness()` for dimmable light/plugin; for RGB light brightness changes the HSV value component. Matter's range maps 0-255 to its valid level representation.
- `color(r,g,b)` for RGB and enhanced color lights; `onColorChange(callback)` reports controller and local color changes.
- `setColorTemperature(mireds)` / `colorTemperature()` use Matter mired units (100–500).
- `setTemperature(C)`, `setHumidity(percent)`, `setContact(closed)`, `setMotion(occupied)`, `setPressure(hPa)`, `setIlluminance(lux)`, `setLeak(bool)`, `setFreeze(bool)`, `setRain(bool)` publish through matching sensor endpoints.
- `sensor("temperature").setValue(x)` and `sensor("humidity").setValue(x)` call those Matter setters for profiles that contain the relevant endpoint.
- Thermostat: `setLocalTemperature(C)`, `setHeatingSetpoint(C)`, `setCoolingSetpoint(C)`, and getters `temperature()`, `heatingSetpoint()`, `coolingSetpoint()`. The application still owns actual heating/cooling logic and safety.
- Fan: `setFanSpeed(0..100)`, `fanSpeed()`, and `setOn()`.
- Window covering: `setLiftPercentage()` updates the measured position; controller target commands are delivered to the callback registered by `onLiftTarget()`. The application must drive a motor and report movement. Percent semantics are 0=open, 100=closed.
- Temperature controlled cabinet: `setCabinetSetpoint(C)` / `cabinetSetpoint()` expose the Matter temperature number endpoint, initially limited to -10..32 °C.
- Generic switch: configure `EasyMatterDeviceType::GenericSwitch`; `button(pin)` sends Matter press/release events from the debounced local input. `switchClick()` emits a standard short click.
- Hardware: `output(pin)`, `pwmOutput(pin, frequency, resolutionBits)`, `setPwm(value)`, `input(pin, mode, debounceMs)`, `analogInput(pin, resolutionBits)`, `analogValue()`, and `button(pin, debounceMs)`. Call `update()` frequently from `loop()`.
- Display: no mandatory driver dependency. Use `display.attach(begin,title,line,show)` to bridge an optional display library.
- Reset: `resetPairing()` calls Espressif `Matter.decommission()`. `resetSettings()` clears only EasyMatter's Preferences namespace. `resetWiFi()` erases Wi-Fi credentials saved by Arduino WiFi. `factoryReset()` performs settings and Wi-Fi erasure, then Matter decommission. These are explicit calls.

## Sensor integration

EasyMatter does not assume GPIO numbers are I2C sensor drivers. Connect your preferred SHT40/SHT31/AHT20/DHT22 library, read it at an interval suitable for the hardware, and call `setTemperature()`/`setHumidity()` with valid measurements. The current generic value holder records validity and update time; use `setRange(min,max)` for application-side bounds. Matter sensor endpoints enforce their own standard ranges. I2C and SPI buses and hardware drivers remain under application control. Analog inputs can be read with Arduino `analogRead()` and translated by the sketch to a supported quantity. Digital GPIO sensors can use `input()` with debounce, or be sampled by sketch code and sent to endpoint setters.

## Compatibility matrix

| Request | Stack capability | EasyMatter implementation |
|---|---|---|
| On/off, dimmable, RGB, tunable-white lights | Official endpoint classes | Implemented profiles |
| On/off/dimmable plug | Official endpoint classes | Implemented profiles |
| Temperature, humidity, pressure, illuminance | Official measurement endpoints | Temperature/humidity and pressure/illuminance profiles/setters implemented (initial defaults for latter two; setters available) |
| Contact, occupancy/motion, leak, freeze, rain | Official Boolean-state endpoints | Implemented |
| Thermostat | Official thermostat endpoint | Endpoint/setpoint APIs implemented; HVAC actuation remains app logic |
| Generic Matter switch | Official switch event endpoint | Implemented as button gestures |
| Fan | Official fan endpoint | Speed/on-off API implemented |
| Window covering, temperature-controlled cabinet | Official endpoint classes | Profiles implemented; motor and cabinet control remain application hardware logic |
| Battery status | No battery-specific endpoint wrapper in this library | Not implemented; requires a proper Power Source cluster integration |
| Smoke alarm, speaker/audio, IR, wearable/tracker, arbitrary vendor metrics | No general matching Arduino endpoint is supplied by the profiles above | Not claimed; requires appropriate standard/custom cluster and product work |
| PWM, relays, LEDs, GPIO inputs | Local Arduino APIs, not Matter clusters by themselves | Basic digital/PWM support; external electrical drivers remain required |
| OLED SSD1306/SH1106 | Separate display libraries | Optional callback bridge, no display driver bundled |

Endpoint availability does not guarantee identical presentation in Alexa or other controllers. The Matter standard and controller determine the available controls. Espressif documents these endpoint classes, their commissioning behavior, and their partition requirements in its [Arduino Matter guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/matter.html), with individual references for [color lights](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/ep_enhanced_color_light.html), [sensors](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/ep_temperature_sensor.html), [thermostats](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/ep_thermostat.html), [fans](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/ep_fan.html), and [generic switches](https://docs.espressif.com/projects/arduino-esp32/en/latest/matter/ep_generic_switch.html).

## Safety and known limits

Never connect mains voltage to an ESP32 GPIO. Use a suitably rated, isolated relay/driver module and enclosure. PWM output is low-voltage logic and does not drive motors directly. GPIO availability and strapping restrictions depend on the ESP32 board; select safe pins. No ADC/I2C/SPI/servo library is bundled. No RAM/flash measurements or hardware tests are claimed. The project environment lacks Arduino CLI/IDE, so it could not compile here; validate against the exact 3.3.12 Matter package and board before deployment.





## API reference example

The `examples/APIReference` sketch documents every public command and configuration field, and provides a Serial Monitor command interface for a dimmable light (`on`, `off`, `toggle`, `brightness`, `status`). Select a different `deviceType` to use the profile-specific APIs described in its header comments.

