#include "EasyMatter.h"
#include <WiFi.h>
#include <Preferences.h>
#include <math.h>
#if __has_include(<qrcode.h>)
#include <qrcode.h>
#define EASYMATTER_HAS_QR_CONSOLE 1
#else
#define EASYMATTER_HAS_QR_CONSOLE 0
#endif

EasyMatterClass EasyMatter;
static Preferences settings;
static bool updateTemperatureValue(void *context, float value) { return static_cast<EasyMatterClass *>(context)->setTemperature(value); }
static bool updateHumidityValue(void *context, float value) { return static_cast<EasyMatterClass *>(context)->setHumidity(value); }
static int hexValue_(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}
static void printOnboardingQRCode_(const String &url) {
  const int dataPosition = url.indexOf("data=");
  if (dataPosition < 0) {
    Serial.printf("Onboarding QR URL: %s\n", url.c_str());
    return;
  }
  String encodedPayload = url.substring(dataPosition + 5);
  const int parameterEnd = encodedPayload.indexOf('&');
  if (parameterEnd >= 0) encodedPayload.remove(parameterEnd);
  String payload;
  payload.reserve(encodedPayload.length());
  for (size_t i = 0; i < encodedPayload.length(); ++i) {
    if (encodedPayload[i] == '%' && i + 2 < encodedPayload.length()) {
      const int high = hexValue_(encodedPayload[i + 1]);
      const int low = hexValue_(encodedPayload[i + 2]);
      if (high >= 0 && low >= 0) {
        payload += char((high << 4) | low);
        i += 2;
        continue;
      }
    }
    payload += encodedPayload[i] == '+' ? ' ' : encodedPayload[i];
  }
#if EASYMATTER_HAS_QR_CONSOLE
  esp_qrcode_config_t qrConfig = ESP_QRCODE_CONFIG_DEFAULT();
  Serial.println("Matter onboarding QR code:");
  if (esp_qrcode_generate(&qrConfig, payload.c_str()) == ESP_OK) return;
  Serial.println("QR rendering failed; use this URL instead:");
#else
  Serial.println("This ESP32 core has no console QR encoder; use this URL:");
#endif
  Serial.println(url);
}
EasyMatterClass::EasyMatterClass() : ready_(false), state_(false), pwmConfigured_(false), wifiRetryAt_(0), brightness_(128), lastRed_(0), lastGreen_(0), lastBlue_(0), colorTemp_(370), callback_(nullptr), brightnessCallback_(nullptr), colorCallback_(nullptr), colorTemperatureCallback_(nullptr), positionCallback_(nullptr), analogPin_(0), analogResolution_(12), pairingResetPin_(0xFF), analogUsed_(false) {
  output_ = {0, OUTPUT, true, false, false, 0, 0, false}; input_ = {0, INPUT_PULLUP, true, false, false, 0, 35, false};
  temperatureValue_.setHandler(updateTemperatureValue, this); humidityValue_.setHandler(updateHumidityValue, this);
}
bool EasyMatterClass::startEndpoint_() {
  switch (config_.deviceType) {
    case EasyMatterDeviceType::Light: return onOff_.begin(config_.initialState);
    case EasyMatterDeviceType::DimmableLight: return dimmable_.begin(config_.initialState, config_.initialBrightness);
    case EasyMatterDeviceType::RgbLight: return rgb_.begin(config_.initialState);
    case EasyMatterDeviceType::ColorTemperatureLight: return colorTempLight_.begin(config_.initialState, config_.initialBrightness, config_.initialColorTemperatureMireds);
    case EasyMatterDeviceType::EnhancedColorLight: return enhanced_.begin(config_.initialState, {21,216,128}, config_.initialBrightness, config_.initialColorTemperatureMireds);
    case EasyMatterDeviceType::Plug: return plug_.begin(config_.initialState);
    case EasyMatterDeviceType::DimmablePlug: return dimmablePlug_.begin(config_.initialState, config_.initialBrightness);
    case EasyMatterDeviceType::TemperatureSensor: return temperatureSensor_.begin(config_.initialTemperatureC);
    case EasyMatterDeviceType::HumiditySensor: return humiditySensor_.begin(config_.initialHumidityPercent);
    case EasyMatterDeviceType::TemperatureHumiditySensor: return temperatureSensor_.begin(config_.initialTemperatureC) && humiditySensor_.begin(config_.initialHumidityPercent);
    case EasyMatterDeviceType::ContactSensor: return contact_.begin();
    case EasyMatterDeviceType::MotionSensor: return motion_.begin(false);
    case EasyMatterDeviceType::Thermostat: return thermostat_.begin(MatterThermostat::THERMOSTAT_SEQ_OP_COOLING_HEATING, MatterThermostat::THERMOSTAT_AUTO_MODE_ENABLED);
    case EasyMatterDeviceType::PressureSensor: return pressureSensor_.begin(1013.25);
    case EasyMatterDeviceType::IlluminanceSensor: return lightSensor_.begin(100.0);
    case EasyMatterDeviceType::WaterLeakSensor: return leak_.begin();
    case EasyMatterDeviceType::WaterFreezeSensor: return freeze_.begin();
    case EasyMatterDeviceType::RainSensor: return rain_.begin();
    case EasyMatterDeviceType::GenericSwitch: return genericSwitch_.begin(MatterGenericSwitch::FEATURE_SIMPLE);
    case EasyMatterDeviceType::Fan: return fan_.begin(0, MatterFan::FAN_MODE_OFF, MatterFan::FAN_MODE_SEQ_OFF_HIGH);
    case EasyMatterDeviceType::WindowCovering: {
      bool ok=window_.begin();
      if(ok) window_.onGoToLiftPercentage([](uint8_t target){ if(EasyMatter.positionCallback_) EasyMatter.positionCallback_(target); return true; });
      return ok;
    }
    case EasyMatterDeviceType::TemperatureControlledCabinet: return cabinet_.begin(config_.initialTemperatureC, -10.0, 32.0, 0.5);
    default: return false;
  }
}
bool EasyMatterClass::begin(const EasyMatterConfig &c) {
  if (!c.wifiSSID || !c.wifiPassword || !c.deviceName || !c.vendor) return false;
  const bool pairingResetRequested = pairingResetPin_ != 0xFF && digitalRead(pairingResetPin_) == LOW;
  config_=c; brightness_=c.initialBrightness; colorTemp_=c.initialColorTemperatureMireds; state_=c.initialState;
  Matter.setVendorName(c.vendor); Matter.setProductName(c.deviceName); Matter.setDeviceName(c.deviceName);
  Serial.printf("EasyMatter 0.3.2\nWiFi: starting\nDevice: %s\nVendor: %s\n",c.deviceName,c.vendor);
#if !CONFIG_ENABLE_CHIPOBLE
  if (!Matter.selectNetwork(MATTER_NETWORK_WIFI, true)) {
    Serial.println("Matter Wi-Fi commissioning setup failed");
    return false;
  }
  WiFi.setAutoReconnect(true);
  matterConnectWiFi(c.wifiSSID,c.wifiPassword);
#endif
  if (!startEndpoint_()) return false;
  Matter.begin(); ready_=true;
  if (pairingResetRequested && Matter.isDeviceCommissioned()) {
    Serial.println("Pairing reset requested on boot; clearing the Matter fabric and restarting");
    Matter.decommission();
    while (true) delay(1000);
  }
  if (pairingResetRequested) {
    Serial.println("Pairing reset pin detected; device is already uncommissioned");
  }
  if(config_.deviceType==EasyMatterDeviceType::RgbLight) setBrightness(c.initialBrightness);
  if (config_.deviceType == EasyMatterDeviceType::TemperatureSensor || config_.deviceType == EasyMatterDeviceType::TemperatureHumiditySensor) setTemperature(c.initialTemperatureC);
  if (config_.deviceType == EasyMatterDeviceType::HumiditySensor || config_.deviceType == EasyMatterDeviceType::TemperatureHumiditySensor) setHumidity(c.initialHumidityPercent);
  if (config_.deviceType == EasyMatterDeviceType::ContactSensor) contact_.setContact(false);
  if (config_.deviceType == EasyMatterDeviceType::Thermostat) { thermostat_.setLocalTemperature(c.initialTemperatureC); thermostat_.setCoolingHeatingSetpoints(20,23); }
  state_=readOn_(); brightness_=brightness(); colorTemp_=colorTemperature();
  if(output_.used){if(pwmConfigured_&&config_.deviceType==EasyMatterDeviceType::Fan)ledcWrite(output_.pin,uint32_t(fanSpeed())*255/100);else writeOutput_(state_);}
  Serial.printf("WiFi: %s\nMatter: Ready\nPairing Code: %s\n",Matter.isWiFiConnected()?"Connected":"connecting",Matter.getManualPairingCode().c_str());
  printOnboardingQRCode_(Matter.getOnboardingQRCodeUrl());
  return true;
}
bool EasyMatterClass::readOn_() const { switch(config_.deviceType) {
 case EasyMatterDeviceType::Light:return const_cast<MatterOnOffLight&>(onOff_).getOnOff(); case EasyMatterDeviceType::DimmableLight:return const_cast<MatterDimmableLight&>(dimmable_).getOnOff();
 case EasyMatterDeviceType::RgbLight:return const_cast<MatterColorLight&>(rgb_).getOnOff(); case EasyMatterDeviceType::ColorTemperatureLight:return const_cast<MatterColorTemperatureLight&>(colorTempLight_).getOnOff();
 case EasyMatterDeviceType::EnhancedColorLight:return const_cast<MatterEnhancedColorLight&>(enhanced_).getOnOff(); case EasyMatterDeviceType::Plug:return const_cast<MatterOnOffPlugin&>(plug_).getOnOff();
 case EasyMatterDeviceType::DimmablePlug:return const_cast<MatterDimmablePlugin&>(dimmablePlug_).getOnOff(); case EasyMatterDeviceType::Fan:return const_cast<MatterFan&>(fan_).getOnOff(); default:return state_; } }
void EasyMatterClass::writeOutput_(bool s) { if(!output_.used)return; if(pwmConfigured_) ledcWrite(output_.pin,s?brightness_:0); else digitalWrite(output_.pin,(s==output_.activeHigh)?HIGH:LOW); }
void EasyMatterClass::notify_(bool s) { if(state_!=s) {state_=s; writeOutput_(s); if(callback_) callback_(s);} }
void EasyMatterClass::update() {
 if (!ready_) return;
 if(!Matter.isWiFiConnected() && int32_t(millis()-wifiRetryAt_)>=0){WiFi.reconnect();wifiRetryAt_=millis()+10000;}
 uint8_t previousBrightness=brightness_; uint16_t previousColorTemp=colorTemp_;
 switch(config_.deviceType) {
 case EasyMatterDeviceType::Light: notify_(onOff_.getOnOff()); break; case EasyMatterDeviceType::DimmableLight: notify_(dimmable_.getOnOff()); brightness_=dimmable_.getBrightness(); break;
 case EasyMatterDeviceType::RgbLight: notify_(rgb_.getOnOff()); brightness_=uint8_t(uint16_t(rgb_.getColorHSV().v)*255/254); break; case EasyMatterDeviceType::ColorTemperatureLight: notify_(colorTempLight_.getOnOff()); brightness_=colorTempLight_.getBrightness(); colorTemp_=colorTempLight_.getColorTemperature(); break;
 case EasyMatterDeviceType::EnhancedColorLight: notify_(enhanced_.getOnOff()); brightness_=enhanced_.getBrightness(); colorTemp_=enhanced_.getColorTemperature(); break;
 case EasyMatterDeviceType::Plug: notify_(plug_.getOnOff()); break; case EasyMatterDeviceType::DimmablePlug: notify_(dimmablePlug_.getOnOff()); brightness_=dimmablePlug_.getLevel(); break; case EasyMatterDeviceType::Fan: notify_(fan_.getOnOff()); brightness_=uint8_t(uint16_t(fan_.getSpeedPercent())*255/100); break; default: break;
 }
 if (brightness_ != previousBrightness) { if(pwmConfigured_) ledcWrite(output_.pin, isOn() ? brightness_ : 0); if(brightnessCallback_) brightnessCallback_(brightness_); }
 if (config_.deviceType==EasyMatterDeviceType::ColorTemperatureLight || config_.deviceType==EasyMatterDeviceType::EnhancedColorLight) { colorTemp_=colorTemperature(); if(previousColorTemp!=colorTemp_ && colorTemperatureCallback_) colorTemperatureCallback_(colorTemp_); }
 if(config_.deviceType==EasyMatterDeviceType::RgbLight || config_.deviceType==EasyMatterDeviceType::EnhancedColorLight) { espRgbColor_t c=config_.deviceType==EasyMatterDeviceType::RgbLight?rgb_.getColorRGB():enhanced_.getColorRGB(); if(colorCallback_ && (c.r!=lastRed_||c.g!=lastGreen_||c.b!=lastBlue_)) colorCallback_(c.r,c.g,c.b);lastRed_=c.r;lastGreen_=c.g;lastBlue_=c.b; }
  if(input_.used) { bool raw=digitalRead(input_.pin); if(raw!=input_.last){input_.last=raw;input_.changed=millis();} if(raw!=input_.stable && millis()-input_.changed>=input_.debounce){input_.stable=raw; const bool pressed=input_.mode==INPUT_PULLUP?raw==LOW:raw==HIGH; if(config_.deviceType==EasyMatterDeviceType::ContactSensor)setContact(pressed); else if(config_.deviceType==EasyMatterDeviceType::MotionSensor)setMotion(pressed); else if(config_.deviceType==EasyMatterDeviceType::GenericSwitch){if(pressed)genericSwitch_.press();else genericSwitch_.release();} else if(pressed)setOn(!isOn()); } }
}
bool EasyMatterClass::setOn(bool s) { if(config_.deviceType==EasyMatterDeviceType::TemperatureSensor||config_.deviceType==EasyMatterDeviceType::HumiditySensor||config_.deviceType==EasyMatterDeviceType::TemperatureHumiditySensor||config_.deviceType==EasyMatterDeviceType::ContactSensor||config_.deviceType==EasyMatterDeviceType::MotionSensor||config_.deviceType==EasyMatterDeviceType::PressureSensor||config_.deviceType==EasyMatterDeviceType::IlluminanceSensor||config_.deviceType==EasyMatterDeviceType::WaterLeakSensor||config_.deviceType==EasyMatterDeviceType::WaterFreezeSensor||config_.deviceType==EasyMatterDeviceType::RainSensor||config_.deviceType==EasyMatterDeviceType::GenericSwitch||config_.deviceType==EasyMatterDeviceType::Thermostat||config_.deviceType==EasyMatterDeviceType::WindowCovering||config_.deviceType==EasyMatterDeviceType::TemperatureControlledCabinet)return false; if(!ready_){state_=s;config_.initialState=s;writeOutput_(s);return true;} bool ok=false; switch(config_.deviceType) {case EasyMatterDeviceType::Light:ok=onOff_.setOnOff(s);break;case EasyMatterDeviceType::DimmableLight:ok=dimmable_.setOnOff(s);break;case EasyMatterDeviceType::RgbLight:ok=rgb_.setOnOff(s);break;case EasyMatterDeviceType::ColorTemperatureLight:ok=colorTempLight_.setOnOff(s);break;case EasyMatterDeviceType::EnhancedColorLight:ok=enhanced_.setOnOff(s);break;case EasyMatterDeviceType::Plug:ok=plug_.setOnOff(s);break;case EasyMatterDeviceType::DimmablePlug:ok=dimmablePlug_.setOnOff(s);break;case EasyMatterDeviceType::Fan:ok=fan_.setOnOff(s);break;default:return false;} if(ok){bool changed=state_!=s;state_=s;if(pwmConfigured_&&config_.deviceType==EasyMatterDeviceType::Fan)ledcWrite(output_.pin,s?uint32_t(fan_.getSpeedPercent())*255/100:0);else writeOutput_(s);if(changed&&callback_)callback_(s);}return ok; }
bool EasyMatterClass::isOn() const { return ready_?readOn_():state_; }
bool EasyMatterClass::setBrightness(uint8_t v) { bool ok=false; switch(config_.deviceType){case EasyMatterDeviceType::DimmableLight:ok=dimmable_.setBrightness(v);break;case EasyMatterDeviceType::ColorTemperatureLight:ok=colorTempLight_.setBrightness(v);break;case EasyMatterDeviceType::EnhancedColorLight:ok=enhanced_.setBrightness(v);break;case EasyMatterDeviceType::DimmablePlug:ok=dimmablePlug_.setLevel(v);break;case EasyMatterDeviceType::RgbLight:{espHsvColor_t h=rgb_.getColorHSV();h.v=(uint16_t(v)*254)/255;ok=rgb_.setColorHSV(h);break;}default:return false;} if(ok){brightness_=v;if(pwmConfigured_)ledcWrite(output_.pin,isOn()?v:0);if(brightnessCallback_)brightnessCallback_(v);}return ok; }
uint8_t EasyMatterClass::brightness() const {switch(config_.deviceType){case EasyMatterDeviceType::DimmableLight:return const_cast<MatterDimmableLight&>(dimmable_).getBrightness();case EasyMatterDeviceType::ColorTemperatureLight:return const_cast<MatterColorTemperatureLight&>(colorTempLight_).getBrightness();case EasyMatterDeviceType::EnhancedColorLight:return const_cast<MatterEnhancedColorLight&>(enhanced_).getBrightness();case EasyMatterDeviceType::DimmablePlug:return const_cast<MatterDimmablePlugin&>(dimmablePlug_).getLevel();case EasyMatterDeviceType::RgbLight:return uint8_t((uint16_t(const_cast<MatterColorLight&>(rgb_).getColorHSV().v)*255)/254);default:return brightness_;}}
bool EasyMatterClass::color(uint8_t r,uint8_t g,uint8_t b){espRgbColor_t rgb={r,g,b}; if(config_.deviceType==EasyMatterDeviceType::RgbLight)return rgb_.setColorRGB(rgb);if(config_.deviceType==EasyMatterDeviceType::EnhancedColorLight)return enhanced_.setColorRGB(rgb);return false;}
bool EasyMatterClass::setColorTemperature(uint16_t v){bool ok=false;if(config_.deviceType==EasyMatterDeviceType::ColorTemperatureLight)ok=colorTempLight_.setColorTemperature(v);else if(config_.deviceType==EasyMatterDeviceType::EnhancedColorLight)ok=enhanced_.setColorTemperature(v);if(ok){colorTemp_=v;if(colorTemperatureCallback_)colorTemperatureCallback_(colorTemp_);}return ok;}
uint16_t EasyMatterClass::colorTemperature() const {if(config_.deviceType==EasyMatterDeviceType::ColorTemperatureLight)return const_cast<MatterColorTemperatureLight&>(colorTempLight_).getColorTemperature();if(config_.deviceType==EasyMatterDeviceType::EnhancedColorLight)return const_cast<MatterEnhancedColorLight&>(enhanced_).getColorTemperature();return colorTemp_;}
bool EasyMatterClass::setTemperature(float v){if(!isfinite(v))return false;bool ok=false;if(config_.deviceType==EasyMatterDeviceType::TemperatureSensor||config_.deviceType==EasyMatterDeviceType::TemperatureHumiditySensor)ok=temperatureSensor_.setTemperature(v);else if(config_.deviceType==EasyMatterDeviceType::Thermostat)ok=thermostat_.setLocalTemperature(v);if(ok)temperatureValue_.setLocalValue(v);return ok;}
float EasyMatterClass::temperature() const {if(config_.deviceType==EasyMatterDeviceType::TemperatureSensor||config_.deviceType==EasyMatterDeviceType::TemperatureHumiditySensor)return const_cast<MatterTemperatureSensor&>(temperatureSensor_).getTemperature();if(config_.deviceType==EasyMatterDeviceType::Thermostat)return const_cast<MatterThermostat&>(thermostat_).getLocalTemperature();return temperatureValue_.value();}
bool EasyMatterClass::setHumidity(float v){if(!isfinite(v)||v<0||v>100)return false;bool ok=(config_.deviceType==EasyMatterDeviceType::HumiditySensor||config_.deviceType==EasyMatterDeviceType::TemperatureHumiditySensor)&&humiditySensor_.setHumidity(v);if(ok)humidityValue_.setLocalValue(v);return ok;}
float EasyMatterClass::humidity() const {if(config_.deviceType==EasyMatterDeviceType::HumiditySensor||config_.deviceType==EasyMatterDeviceType::TemperatureHumiditySensor)return const_cast<MatterHumiditySensor&>(humiditySensor_).getHumidity();return humidityValue_.value();}
bool EasyMatterClass::setContact(bool v){if(config_.deviceType==EasyMatterDeviceType::ContactSensor)return contact_.setContact(v);return false;} bool EasyMatterClass::setMotion(bool v){if(config_.deviceType==EasyMatterDeviceType::MotionSensor)return motion_.setOccupancy(v);return false;}
bool EasyMatterClass::isContactClosed()const{return config_.deviceType==EasyMatterDeviceType::ContactSensor?const_cast<MatterContactSensor&>(contact_).getContact():false;} bool EasyMatterClass::isMotionDetected()const{return config_.deviceType==EasyMatterDeviceType::MotionSensor?const_cast<MatterOccupancySensor&>(motion_).getOccupancy():false;}
bool EasyMatterClass::setPressure(float v){return config_.deviceType==EasyMatterDeviceType::PressureSensor&&isfinite(v)&&pressureSensor_.setPressure(v);}
bool EasyMatterClass::setIlluminance(float v){return config_.deviceType==EasyMatterDeviceType::IlluminanceSensor&&isfinite(v)&&lightSensor_.setIlluminance(v);}
float EasyMatterClass::pressure()const{return config_.deviceType==EasyMatterDeviceType::PressureSensor?const_cast<MatterPressureSensor&>(pressureSensor_).getPressure():NAN;} float EasyMatterClass::illuminance()const{return config_.deviceType==EasyMatterDeviceType::IlluminanceSensor?const_cast<MatterLightSensor&>(lightSensor_).getIlluminance():NAN;}
bool EasyMatterClass::setLeak(bool v){return config_.deviceType==EasyMatterDeviceType::WaterLeakSensor&&leak_.setLeak(v);}
bool EasyMatterClass::setFreeze(bool v){return config_.deviceType==EasyMatterDeviceType::WaterFreezeSensor&&freeze_.setFreeze(v);}
bool EasyMatterClass::setRain(bool v){return config_.deviceType==EasyMatterDeviceType::RainSensor&&rain_.setRain(v);}
bool EasyMatterClass::isLeakDetected()const{return config_.deviceType==EasyMatterDeviceType::WaterLeakSensor?const_cast<MatterWaterLeakDetector&>(leak_).getLeak():false;} bool EasyMatterClass::isFreezeDetected()const{return config_.deviceType==EasyMatterDeviceType::WaterFreezeSensor?const_cast<MatterWaterFreezeDetector&>(freeze_).getFreeze():false;} bool EasyMatterClass::isRaining()const{return config_.deviceType==EasyMatterDeviceType::RainSensor?const_cast<MatterRainSensor&>(rain_).getRain():false;}
void EasyMatterClass::switchClick(){if(config_.deviceType==EasyMatterDeviceType::GenericSwitch)genericSwitch_.click();}
bool EasyMatterClass::setLocalTemperature(float v){return config_.deviceType==EasyMatterDeviceType::Thermostat?thermostat_.setLocalTemperature(v):false;}
bool EasyMatterClass::setHeatingSetpoint(float v){return config_.deviceType==EasyMatterDeviceType::Thermostat?thermostat_.setHeatingSetpoint(v):false;} bool EasyMatterClass::setCoolingSetpoint(float v){return config_.deviceType==EasyMatterDeviceType::Thermostat?thermostat_.setCoolingSetpoint(v):false;}
float EasyMatterClass::heatingSetpoint()const{return config_.deviceType==EasyMatterDeviceType::Thermostat?const_cast<MatterThermostat&>(thermostat_).getHeatingSetpoint():NAN;} float EasyMatterClass::coolingSetpoint()const{return config_.deviceType==EasyMatterDeviceType::Thermostat?const_cast<MatterThermostat&>(thermostat_).getCoolingSetpoint():NAN;}
bool EasyMatterClass::setFanSpeed(uint8_t v){if(config_.deviceType!=EasyMatterDeviceType::Fan||v>100)return false;bool ok=fan_.setSpeedPercent(v);if(ok&&pwmConfigured_)ledcWrite(output_.pin,isOn()?uint32_t(v)*255/100:0);return ok;} uint8_t EasyMatterClass::fanSpeed()const{return config_.deviceType==EasyMatterDeviceType::Fan?const_cast<MatterFan&>(fan_).getSpeedPercent():0;}
bool EasyMatterClass::setLiftPercentage(uint8_t v){return config_.deviceType==EasyMatterDeviceType::WindowCovering&&v<=100&&window_.setLiftPercentage(v);} uint8_t EasyMatterClass::liftPercentage()const{return config_.deviceType==EasyMatterDeviceType::WindowCovering?const_cast<MatterWindowCovering&>(window_).getLiftPercentage():0;} void EasyMatterClass::onLiftTarget(PositionCallback cb){positionCallback_=cb;}
bool EasyMatterClass::setCabinetSetpoint(float v){return config_.deviceType==EasyMatterDeviceType::TemperatureControlledCabinet&&isfinite(v)&&cabinet_.setTemperatureSetpoint(v);} float EasyMatterClass::cabinetSetpoint()const{return config_.deviceType==EasyMatterDeviceType::TemperatureControlledCabinet?const_cast<MatterTemperatureControlledCabinet&>(cabinet_).getTemperatureSetpoint():NAN;}
bool EasyMatterClass::output(uint8_t p,uint8_t m,bool ah){pinMode(p,m);output_={p,m,ah,false,false,0,0,true};pwmConfigured_=false;writeOutput_(state_);return true;}
bool EasyMatterClass::pwmOutput(uint8_t p,uint32_t hz,uint8_t bits){if(!ledcAttach(p,hz,bits))return false;output_={p,OUTPUT,true,false,false,0,0,true};pwmConfigured_=true;ledcWrite(p,isOn()?brightness_:0);return true;}
bool EasyMatterClass::setPwm(uint8_t v){if(!output_.used||!pwmConfigured_)return false;return ledcWrite(output_.pin,v);}
bool EasyMatterClass::input(uint8_t p,uint8_t m,uint16_t d){pinMode(p,m);bool raw=digitalRead(p);input_={p,m,true,raw,raw,millis(),d,true};return true;}void EasyMatterClass::button(uint8_t p,uint16_t d){input(p,INPUT_PULLUP,d);}
void EasyMatterClass::pairingResetButton(uint8_t p){pairingResetPin_=p;pinMode(p,INPUT_PULLUP);}
bool EasyMatterClass::analogInput(uint8_t p,uint8_t bits){if(bits<9||bits>12)return false;analogReadResolution(bits);analogPin_=p;analogResolution_=bits;analogUsed_=true;return true;}uint16_t EasyMatterClass::analogValue()const{return analogUsed_?analogRead(analogPin_):0;}
void EasyMatterClass::onChange(ChangeCallback c){callback_=c;}
void EasyMatterClass::onBrightnessChange(BrightnessCallback c){brightnessCallback_=c;} void EasyMatterClass::onColorChange(ColorCallback c){colorCallback_=c;}
void EasyMatterClass::onColorTemperatureChange(ColorTemperatureCallback c){colorTemperatureCallback_=c;}
bool EasyMatterClass::isCommissioned() const{return ready_&&Matter.isDeviceCommissioned();}const char* EasyMatterClass::pairingCode()const{static String s;s=Matter.getManualPairingCode();return s.c_str();}const char*EasyMatterClass::qrCodeUrl()const{static String s;s=Matter.getOnboardingQRCodeUrl();return s.c_str();}
EasyMatterSensorValue&EasyMatterClass::sensor(const char*n){return n&&strcmp(n,"humidity")==0?humidityValue_:temperatureValue_;}
bool EasyMatterClass::resetPairing(){if(!ready_)return false;Matter.decommission();return true;}bool EasyMatterClass::resetSettings(){if(!settings.begin("easymatter",false))return false;bool ok=settings.clear();settings.end();return ok;}bool EasyMatterClass::resetWiFi(){WiFi.disconnect(true,true);return true;}bool EasyMatterClass::factoryReset(){bool a=resetSettings(),b=resetWiFi(),c=resetPairing();return a&&b&&c;}




