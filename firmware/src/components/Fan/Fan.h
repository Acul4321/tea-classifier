#ifndef FAN_H
#define FAN_H

#include <Arduino.h>

class Fan {
public:
  explicit Fan(uint8_t pin);

  void begin();
  void runFor(unsigned long durationMs);
  void on();
  void off();
  void update();

private:
  uint8_t pin_;
  unsigned long startedAt_;
  unsigned long durationMs_;
  bool durationActive_;
  bool isOn_;
};

#endif