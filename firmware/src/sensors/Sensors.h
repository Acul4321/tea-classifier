#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

struct BME688Data {
  bool valid = false;
  float temperature = 0.0f;
  float pressure = 0.0f;
  float humidity = 0.0f;
  float gasResistance = 0.0f;
  float altitude = 0.0f;
};

struct SHT41Data {
  bool valid = false;
  float temperature = 0.0f;
  float humidity = 0.0f;
};

struct SGP41Data {
  bool valid = false;
  uint16_t vocRaw = 0;
  uint16_t noxRaw = 0;
};

struct AS7341Data {
  bool valid = false;
  uint16_t channels[12] = {};
};

struct CJMCU6814Data {
  bool valid = false;
  bool ready = false;
  float co = 0.0f;
  float nh3 = 0.0f;
  float no2 = 0.0f;
};

struct AllSensorData {
  BME688Data bme688;
  SHT41Data sht41;
  SGP41Data sgp41;
  AS7341Data as7341;
  CJMCU6814Data cjmcu6814;
};

void sensor_init();
void initBME688();
void initSGP41();
void initSHT41();
void initAS7341();
void initCJMCU6814();

void scanI2C();
bool cjmcuReady();
BME688Data readBME688();
SHT41Data readSHT41();
SGP41Data readSGP41();
AS7341Data readAS7341();
CJMCU6814Data readCJMCU6814();
AllSensorData readAllSensors();

#endif
