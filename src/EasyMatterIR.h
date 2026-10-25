#pragma once
#include <Arduino.h>

class EasyMatterIR {
 public:
  typedef bool (*BeginHandler)(void *context);
  typedef bool (*SendHandler)(void *context, uint32_t protocol, uint32_t code, uint8_t bits);
  EasyMatterIR() : beginHandler_(nullptr), sendHandler_(nullptr), context_(nullptr) {}
  void attach(BeginHandler beginHandler, SendHandler sendHandler, void *context = nullptr) {
    beginHandler_ = beginHandler; sendHandler_ = sendHandler; context_ = context;
  }
  bool begin() { return beginHandler_ && beginHandler_(context_); }
  bool send(uint32_t protocol, uint32_t code, uint8_t bits) {
    return sendHandler_ && sendHandler_(context_, protocol, code, bits);
  }
 private:
  BeginHandler beginHandler_; SendHandler sendHandler_; void *context_;
};
