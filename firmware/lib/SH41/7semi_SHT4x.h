/********************************************************
 * @file 7semi_SHT4x.h
 *
 * @mainpage 7semi SHT4x Temperature and Humidity Sensor Library
 *
 * @section intro_sec Introduction
 *
 * This is the Arduino library for the Sensirion SHT4x family of digital 
 * temperature and humidity sensors (SHT40, SHT41, SHT45).
 *
 * It allows users to easily configure precision, read sensor data, and 
 * control the internal heater functions using I2C communication.
 *
 * This library is developed and maintained by 7semi, intended for prototyping
 * and evaluation purposes. It may not be fully calibrated or accurate for
 * critical applications.
 *
 * @section features Features
 * - I2C-based communication
 * - Read temperature and relative humidity
 * - Select measurement repeatability (precision)
 * - Internal heater support (multiple power/time modes)
 * - Read serial number
 * - Perform software reset
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


#ifndef _7SEMI_SHT4X_H_
#define _7SEMI_SHT4X_H_

#include <Arduino.h>
#include <Wire.h>
// Repeatability settings for sensor measurements
// Trade-off: higher repeatability = more accurate, but slower and higher power use
enum Repeatability {
    REPEATABILITY_LOW    = 0xE0,  // Fastest, lowest power, less accurate
    REPEATABILITY_MEDIUM = 0xF6,  // Balanced choice (recommended for most use cases)
    REPEATABILITY_HIGH   = 0xFD   // Slowest, highest power, most accurate
};

// Heater settings for the built-in sensor heater.
// Format: POWER (in milliwatts) + duration (in seconds).
    enum HeaterPower {
        HEATER_20mW_0_1s   = 0x15,  // Heater: 20 mW power, 0.1 second duration
        HEATER_20mW_1s     = 0x1E,  // Heater: 20 mW power, 1 second duration
        HEATER_110mW_0_1s  = 0x24,  // Heater: 110 mW power, 0.1 second duration
        HEATER_110mW_1s    = 0x2F,  // Heater: 110 mW power, 1 second duration
        HEATER_200mW_0_1s  = 0x32,  // Heater: 200 mW power, 0.1 second duration
        HEATER_200mW_1s    = 0x39   // Heater: 200 mW power, 1 second duration
    };
    
class SHT4x_7semi {
public:
    SHT4x_7semi(uint8_t address = 0x44);

    bool begin(TwoWire& wirePort = Wire);                            // zero-arg OK
bool begin(TwoWire& wirePort, int sda, int scl, uint32_t freq = 0); // must pass wirePort

    // Main measurement (optional precision override)
    bool readTemperatureHumidity(float& temperature, float& humidity, Repeatability precision);
    bool readTemperatureHumidity(float& temperature, float& humidity); // uses setPrecision()

    // Configuration
    void setPrecision(Repeatability precision);
    bool setHeater(HeaterPower mode);

    // Utility
    bool readSerialNumber(uint32_t& serial);
    bool softReset();
    bool heaterMeasurement(float& temperature, float& humidity, HeaterPower mode);


private:
    uint8_t _i2c_address;
    TwoWire* _wire;
    Repeatability _precision;

    bool writeCommand(uint8_t cmd);
    bool readResponse(uint8_t* buffer, uint8_t len);
    uint8_t calculateCRC(const uint8_t* data, uint8_t len);
};

#endif
