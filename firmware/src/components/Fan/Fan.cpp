#include "Fan.h"

Fan::Fan(uint8_t pin)
    : pin_(pin),
  startedAt_(0),
  durationMs_(0),
  durationActive_(false),
      isOn_(false) {
}

void Fan::begin() {
  pinMode(pin_, OUTPUT);
  off();
}

void Fan::runFor(unsigned long durationMs) {
  if (durationMs == 0) {
    off();
    return;
  }

  startedAt_ = millis();
  durationMs_ = durationMs;
  durationActive_ = true;
  isOn_ = true;
  digitalWrite(pin_, HIGH);
}

void Fan::on() {
  durationActive_ = false;
  isOn_ = true;
  digitalWrite(pin_, HIGH);
}

void Fan::off() {
  durationActive_ = false;
  isOn_ = false;
  digitalWrite(pin_, LOW);
}

void Fan::update() {
  if (!durationActive_ || !isOn_) {
    return;
  }

  if (millis() - startedAt_ >= durationMs_) {
    off();
  }
}

bool Fan::isOn() const {
  return isOn_;
}