#pragma once
#include <Arduino.h>
#include <math.h>

class EasyMatterSensorValue {
 public:
  typedef bool (*SetHandler)(void *, float);
  EasyMatterSensorValue() : value_(NAN), valid_(false), minimum_(NAN), maximum_(NAN), updatedAt_(0), handler_(nullptr), context_(nullptr) {}
  bool setValue(float value) { if (!isfinite(value) || (isfinite(minimum_) && value < minimum_) || (isfinite(maximum_) && value > maximum_)) { valid_ = false; return false; } if (handler_) return handler_(context_, value); setLocalValue(value); return true; }
  void setRange(float minimum, float maximum) { minimum_ = minimum; maximum_ = maximum; }
  void setHandler(SetHandler handler, void *context) { handler_ = handler; context_ = context; }
  void setLocalValue(float value) { value_ = value; valid_ = isfinite(value); updatedAt_ = millis(); }
  bool valid() const { return valid_; }
  float value() const { return value_; }
  uint32_t updatedAt() const { return updatedAt_; }
 private:
  float value_; bool valid_; float minimum_, maximum_; uint32_t updatedAt_; SetHandler handler_; void *context_;
};

