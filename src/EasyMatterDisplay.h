#pragma once
#include <Arduino.h>

class EasyMatterDisplay {
 public:
  typedef void (*BeginFn)();
  typedef void (*TitleFn)(const char *);
  typedef void (*LineFn)(uint8_t, const char *);
  typedef void (*ShowFn)();
  EasyMatterDisplay() : beginFn_(nullptr), titleFn_(nullptr), lineFn_(nullptr), showFn_(nullptr) {}
  void attach(BeginFn b, TitleFn t, LineFn l, ShowFn s) { beginFn_=b; titleFn_=t; lineFn_=l; showFn_=s; }
  bool begin() { if (!beginFn_) return false; beginFn_(); return true; }
  void title(const char *s) { if (titleFn_) titleFn_(s); }
  void line(uint8_t n, const char *s) { if (lineFn_) lineFn_(n,s); }
  void show() { if (showFn_) showFn_(); }
 private: BeginFn beginFn_; TitleFn titleFn_; LineFn lineFn_; ShowFn showFn_;
};
