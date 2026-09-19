#ifndef CJMCU6814_H
#define CJMCU6814_H

#include <Arduino.h>

/**
 * @class CJMCU6814
 * @brief Driver for CJMCU-6814 multi-gas sensor (CO, NH3, NO2)
 * 
 * Provides analog ADC reading and PPM conversion using datasheet
 * curve-fitting equations and R0 calibration baseline.
 */
class CJMCU6814 {
public:
  /**
   * @brief Constructor
   * 
   * Initializes with default pin assignments (CO=0, NH3=1, NO2=3)
   * and default R0 calibration values for clean air:
   * - CO: 750 kΩ
   * - NH3: 500 kΩ
   * - NO2: 22 kΩ
   */
  CJMCU6814();

  /**
   * @brief Initialize the sensor hardware and configure pins
   * 
   * Sets ADC attenuation to 11dB for full 0-3.3V range on ESP32-C3
   * and stores the GPIO pin assignments for gas sensors.
   * 
   * @param pin_co   GPIO pin for CO sensor (default: 0)
   * @param pin_nh3  GPIO pin for NH3 sensor (default: 1)
   * @param pin_no2  GPIO pin for NO2 sensor (default: 3)
   */
  void begin(int pin_co = 0, int pin_nh3 = 1, int pin_no2 = 3);

  /**
   * @brief Read and calculate gas concentrations
   * 
   * @param[out] co   Carbon monoxide concentration in PPM
   * @param[out] nh3  Ammonia concentration in PPM
   * @param[out] no2  Nitrogen dioxide concentration in PPM
   */
  void readGases(float& co, float& nh3, float& no2);

  /**
   * @brief Set custom R0 baseline calibration values
   * 
   * @param r0_co   R0 value for CO channel (ohms)
   * @param r0_nh3  R0 value for NH3 channel (ohms)
   * @param r0_no2  R0 value for NO2 channel (ohms)
   */
  void setR0(float r0_co, float r0_nh3, float r0_no2);

private:
  int pin_co_;
  int pin_nh3_;
  int pin_no2_;

  float r0_co_;
  float r0_nh3_;
  float r0_no2_;

  /**
   * @brief Calculate sensor element resistance from ADC reading
   * 
   * @param pin ADC pin to read
   * @return Sensor resistance in ohms
   */
  float getResistance(int pin);

  /**
   * @brief Convert resistance ratio to PPM using datasheet curves
   * 
   * @param rs       Current sensor resistance (ohms)
   * @param r0       Reference baseline resistance (ohms)
   * @param gasType  0=CO, 1=NH3, 2=NO2
   * @return Estimated concentration in PPM
   */
  float getGasPPM(float rs, float r0, int gasType);
};

#endif
