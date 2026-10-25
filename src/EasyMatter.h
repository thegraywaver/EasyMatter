#pragma once
#include <Arduino.h>
#include "EasyMatterConfig.h"
#include "EasyMatterSensor.h"
#include "EasyMatterDisplay.h"
#include "EasyMatterIR.h"
#if !defined(ARDUINO_ARCH_ESP32)
#error "EasyMatter requires Espressif Arduino-ESP32 and its official Matter library."
#endif
#include <Matter.h>

class EasyMatterMountedSwitchEndpoint : public MatterOnOffPlugin, public ArduinoMatter {
 public:
  bool beginMountedSwitch(bool initialState);
};

class EasyMatterClass {
 public:
  typedef void (*ChangeCallback)(bool state);
  typedef void (*BrightnessCallback)(uint8_t brightness);
  typedef void (*ColorCallback)(uint8_t red, uint8_t green, uint8_t blue);
  typedef void (*ColorTemperatureCallback)(uint16_t mireds);
  typedef void (*PositionCallback)(uint8_t targetPercent);
  EasyMatterClass();
  bool begin(const EasyMatterConfig &config);
  void update();
  bool setOn(bool on); bool isOn() const;
  bool setBrightness(uint8_t value); uint8_t brightness() const;
  bool color(uint8_t red, uint8_t green, uint8_t blue);
  bool setColorTemperature(uint16_t mireds); uint16_t colorTemperature() const;
  bool setTemperature(float celsius); float temperature() const;
  bool setHumidity(float percent); float humidity() const;
  bool setContact(bool closed); bool setMotion(bool occupied);
  bool isContactClosed() const; bool isMotionDetected() const;
  bool setPressure(float hectopascals); bool setIlluminance(float lux);
  float pressure() const; float illuminance() const;
  bool setLeak(bool detected); bool setFreeze(bool detected); bool setRain(bool detected);
  bool isLeakDetected() const; bool isFreezeDetected() const; bool isRaining() const;
  void switchClick();
  bool setLocalTemperature(float celsius); bool setHeatingSetpoint(float celsius); bool setCoolingSetpoint(float celsius);
  float heatingSetpoint() const; float coolingSetpoint() const;
  bool setFanSpeed(uint8_t percent); uint8_t fanSpeed() const;
  bool setLiftPercentage(uint8_t percent); uint8_t liftPercentage() const; void onLiftTarget(PositionCallback callback);
  bool setCabinetSetpoint(float celsius); float cabinetSetpoint() const;
  bool output(uint8_t pin, uint8_t mode = OUTPUT, bool activeHigh = true);
  bool pwmOutput(uint8_t pin, uint32_t frequency = 5000, uint8_t resolutionBits = 8);
  bool setPwm(uint8_t value);
  bool input(uint8_t pin, uint8_t mode = INPUT_PULLUP, uint16_t debounceMs = 35);
  bool analogInput(uint8_t pin, uint8_t resolutionBits = 12); uint16_t analogValue() const;
  void button(uint8_t pin, uint16_t debounceMs = 35);
  void pairingResetButton(uint8_t pin);
  void onChange(ChangeCallback callback);
  void onBrightnessChange(BrightnessCallback callback); void onColorChange(ColorCallback callback);
  void onColorTemperatureChange(ColorTemperatureCallback callback);
  bool resetPairing(); bool resetSettings(); bool resetWiFi(); bool factoryReset();
  bool isReady() const { return ready_; }
  bool isCommissioned() const;
  const char *pairingCode() const; const char *qrCodeUrl() const;
  EasyMatterSensorValue &sensor(const char *name);
  EasyMatterDisplay display;
  EasyMatterIR ir;
 private:
  struct Pin { uint8_t pin, mode; bool activeHigh, stable, last; uint32_t changed; uint16_t debounce; bool used; };
  bool startEndpoint_(); bool readOn_() const; void writeOutput_(bool state); void notify_(bool state); void persistSwitchState_(bool state);
  EasyMatterConfig config_; bool ready_, state_, pwmConfigured_; uint32_t wifiRetryAt_; uint8_t brightness_, lastRed_, lastGreen_, lastBlue_; uint16_t colorTemp_; ChangeCallback callback_; BrightnessCallback brightnessCallback_; ColorCallback colorCallback_; ColorTemperatureCallback colorTemperatureCallback_; PositionCallback positionCallback_;
  Pin output_, input_; uint8_t analogPin_, analogResolution_, pairingResetPin_; bool analogUsed_; EasyMatterSensorValue temperatureValue_, humidityValue_;
  MatterOnOffLight onOff_; MatterDimmableLight dimmable_; MatterColorLight rgb_;
  MatterColorTemperatureLight colorTempLight_; MatterEnhancedColorLight enhanced_; MatterOnOffPlugin plug_; EasyMatterMountedSwitchEndpoint switchEndpoint_; MatterDimmablePlugin dimmablePlug_;
  MatterTemperatureSensor temperatureSensor_; MatterHumiditySensor humiditySensor_; MatterContactSensor contact_; MatterOccupancySensor motion_; MatterThermostat thermostat_;
  MatterPressureSensor pressureSensor_; MatterLightSensor lightSensor_; MatterWaterLeakDetector leak_; MatterWaterFreezeDetector freeze_; MatterRainSensor rain_; MatterGenericSwitch genericSwitch_; MatterFan fan_;
  MatterWindowCovering window_; MatterTemperatureControlledCabinet cabinet_;
};
extern EasyMatterClass EasyMatter;
