#include "CJMCU6814.h"

CJMCU6814::CJMCU6814()
    : pin_co_(0),
      pin_nh3_(1),
      pin_no2_(3),
      r0_co_(750000.0),
      r0_nh3_(500000.0),
      r0_no2_(22000.0) {
}

void CJMCU6814::begin(int pin_co, int pin_nh3, int pin_no2) {
  pin_co_ = pin_co;
  pin_nh3_ = pin_nh3;
  pin_no2_ = pin_no2;
  
  analogSetAttenuation(ADC_11db);
}

float CJMCU6814::getResistance(int pin) {
  int rawADC = analogRead(pin);
  if (rawADC == 0) rawADC = 1;
  float voltage = (rawADC * 3.3) / 4095.0;
  float rs = ((3.3 - voltage) / voltage) * 10000.0;
  return rs;
}

float CJMCU6814::getGasPPM(float rs, float r0, int gasType) {
  float ratio = rs / r0;
  if (ratio <= 0) return 0.0;

  switch (gasType) {
    case 0:  // CO
      return pow(10, -1.179 * log10(ratio) + 0.605);
    case 1:  // NH3
      return pow(10, -1.8 * log10(ratio) + 0.82);
    case 2:  // NO2
      return pow(10, 1.008 * log10(ratio) + 0.12);
    default:
      return 0.0;
  }
}

void CJMCU6814::readGases(float& co, float& nh3, float& no2) {
  float rs_co = getResistance(pin_co_);
  float rs_nh3 = getResistance(pin_nh3_);
  float rs_no2 = getResistance(pin_no2_);

  co = getGasPPM(rs_co, r0_co_, 0);
  nh3 = getGasPPM(rs_nh3, r0_nh3_, 1);
  no2 = getGasPPM(rs_no2, r0_no2_, 2);
}

void CJMCU6814::setR0(float r0_co, float r0_nh3, float r0_no2) {
  r0_co_ = r0_co;
  r0_nh3_ = r0_nh3;
  r0_no2_ = r0_no2;
}