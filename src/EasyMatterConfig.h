#pragma once
#include <Arduino.h>

enum class EasyMatterDeviceType : uint8_t {
  Light, DimmableLight, RgbLight, ColorTemperatureLight, EnhancedColorLight,
  Plug, DimmablePlug,
  TemperatureSensor, HumiditySensor, TemperatureHumiditySensor,
  ContactSensor, MotionSensor, Thermostat, PressureSensor, IlluminanceSensor,
  WaterLeakSensor, WaterFreezeSensor, RainSensor, GenericSwitch, Fan,
  WindowCovering, TemperatureControlledCabinet
};

struct EasyMatterConfig {
  const char *wifiSSID;
  const char *wifiPassword;
  const char *vendor;
  const char *deviceName;
  EasyMatterDeviceType deviceType;
  bool debug;
  bool initialState;
  uint8_t initialBrightness;
  uint16_t initialColorTemperatureMireds;
  float initialTemperatureC;
  float initialHumidityPercent;
  EasyMatterConfig() : wifiSSID(nullptr), wifiPassword(nullptr), vendor("EasyMatter"),
    deviceName("EasyMatter device"), deviceType(EasyMatterDeviceType::Light), debug(false), initialState(false),
    initialBrightness(128), initialColorTemperatureMireds(370), initialTemperatureC(20.0f), initialHumidityPercent(50.0f) {}
};
