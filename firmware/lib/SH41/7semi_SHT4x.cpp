/********************************************************
 * @file 7semi_SHT4x.cpp
 *
 * @mainpage 7semi SHT4x Temperature and Humidity Sensor Library
 *
 * @section intro_sec Introduction
 *
 * This is the Arduino library for the Sensirion SHT4x family of temperature
 * and humidity sensors (SHT40, SHT41, SHT45).
 * 
 * It supports reading temperature and relative humidity with optional
 * configurable precision and onboard heater modes.
 *
 * Communication is done via I2C, requiring only two pins (SCL and SDA).
 * This library simplifies interaction with the sensor and handles CRC checks,
 * raw data conversion, and configuration commands.
 *
 * This library is developed and maintained by 7semi, intended for prototyping
 * and evaluation purposes. It may not be fully calibrated or accurate for
 * critical applications.
 *
 * @section features Features
 * - Read temperature and humidity values
 * - Selectable measurement precision (low, medium, high)
 * - Internal heater support with multiple power levels
 * - Read device serial number
 * - Soft reset command
 *
 * @section author Author
 *
 * Developed by 7semi.
 *
 * @section license License
 *
 * @license MIT
 * Copyright (c) 2025 7semi
 ******************************************************/


#include "7semi_SHT4x.h"

SHT4x_7semi::SHT4x_7semi(uint8_t address)
  : _i2c_address(address), _wire(&Wire), _precision(REPEATABILITY_HIGH) {}

bool SHT4x_7semi::begin(TwoWire& wirePort) {
  _wire = &wirePort;
  _wire->begin();
  uint32_t sn;
  return readSerialNumber(sn); // If SN read is successful, sensor is present
}

bool SHT4x_7semi::writeCommand(uint8_t cmd) {
  _wire->beginTransmission(_i2c_address);
  _wire->write(cmd);
  return _wire->endTransmission() == 0;
}
// ESP32-friendly: use custom pins if provided, otherwise fall back to defaults
bool SHT4x_7semi::begin(TwoWire& wirePort, int sda /* = -1 */, int scl /* = -1 */, uint32_t freq /* = 0 */) {
  _wire = &wirePort;

#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 allows custom SDA/SCL pins
  if (sda >= 0 && scl >= 0) {
    if (freq > 0) _wire->begin(sda, scl, freq);  // custom pins + custom freq
    else          _wire->begin(sda, scl);        // custom pins + default freq
  } else {
    if (freq > 0) {
      _wire->begin();        // default pins
      _wire->setClock(freq); // custom freq
    } else {
      _wire->begin();        // default pins + default freq
    }
  }
#else
  // AVR, SAMD, etc. → pins fixed by hardware
  (void)sda; (void)scl;
  _wire->begin();            
  if (freq > 0) _wire->setClock(freq);
#endif

  uint32_t sn;
  return readSerialNumber(sn);
}


bool SHT4x_7semi::readResponse(uint8_t* buffer, uint8_t len) {
  if (_wire->requestFrom(_i2c_address, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) {
    buffer[i] = _wire->read();
  }
  return true;
}

uint8_t SHT4x_7semi::calculateCRC(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0xFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++)
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
  }
  return crc;
}

bool SHT4x_7semi::readTemperatureHumidity(float& temperature, float& humidity, Repeatability precision) {
  if (!writeCommand(precision)) return false;
  delay(10);
  uint8_t data[6];
  if (!readResponse(data, 6)) return false;

  if (calculateCRC(data, 2) != data[2] || calculateCRC(data + 3, 2) != data[5]) return false;

  uint16_t tempRaw = (data[0] << 8) | data[1];
  uint16_t humRaw = (data[3] << 8) | data[4];

  temperature = -45.0f + 175.0f * tempRaw / 65535.0f;
  humidity = -6.0f + 125.0f * humRaw / 65535.0f;
  humidity = constrain(humidity, 0.0f, 100.0f);

  return true;
}

// Overload: uses the internally stored precision
bool SHT4x_7semi::readTemperatureHumidity(float& temperature, float& humidity) {
  return readTemperatureHumidity(temperature, humidity, _precision);
}

bool SHT4x_7semi::heaterMeasurement(float& temperature, float& humidity, HeaterPower mode) {
  if (!writeCommand(mode)) return false;
  delay(1100);
  return readTemperatureHumidity(temperature, humidity, REPEATABILITY_HIGH);
}
bool SHT4x_7semi::setHeater(HeaterPower mode) {
  if (!writeCommand(mode)) return false;
  return true;
}
bool SHT4x_7semi::readSerialNumber(uint32_t& serial) {
  if (!writeCommand(0x89)) return false;
  delay(1);
  uint8_t data[6];
  if (!readResponse(data, 6)) return false;

  if (calculateCRC(data, 2) != data[2] || calculateCRC(data + 3, 2) != data[5]) return false;

  serial = (uint32_t)data[0] << 24 | (uint32_t)data[1] << 16 | (uint32_t)data[3] << 8 | data[4];
  return true;
}

bool SHT4x_7semi::softReset() {
  return writeCommand(0x94);
}

void SHT4x_7semi::setPrecision(Repeatability precision) {
  _precision = precision;
}
