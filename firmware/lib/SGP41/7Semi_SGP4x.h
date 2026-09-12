#ifndef _7SEMI_SGP4X_H_
#define _7SEMI_SGP4X_H_

#include <Arduino.h>
#include <Wire.h>

/**
 * Command Definitions
 *
 * - Direct datasheet command values
 * - Used internally by driver
 */
#define CMD_SGP40_MEASURE_RAW    0x260F
#define CMD_SGP41_MEASURE_RAW    0x2619
#define CMD_SGP41_CONDITIONING   0x2612
#define CMD_SGP4X_SERIAL_NUMBER  0x3682
#define CMD_HEATER_OFF           0x3615


/**
 * Datasheet Timing Delays
 *
 * - Required wait times after measurement commands
 * - Values based on typical datasheet timing
 */
static constexpr uint16_t SGP41_DELAY_MS  = 50;
static constexpr uint16_t SGP40_DELAY_MS  = 30;
static constexpr uint16_t SERIAL_DELAY_MS = 2;


/**
 * 7Semi SGP4x Gas Sensor Driver
 *
 * - Supports SGP40 and SGP41
 * - No background task engine
 * - No automatic conditioning scheduler
 * - Heater stays ON unless explicitly disabled
 *
 * Design Philosophy
 * - Beginner friendly: simple readVOC()
 * - Professional friendly: full manual control
 */
class SGP4x_7Semi
{
public:

    /**
     * Supported sensor types
     *
     * - SGP40_SENSOR : VOC only
     * - SGPT41 : VOC + NOx
     */
    enum SGP4x : uint8_t
    {
        SGP40_SENSOR = 0,
        SGPT41
    };


    /**
     * Default compensation values
     *
     * - 50 percent relative humidity
     * - 25 degrees Celsius
     * - Used when no external compensation provided
     */
    static constexpr uint16_t default_humidity_ticks    = 0x8000;
    static constexpr uint16_t default_temperature_ticks = 0x6666;


    /**
     * Constructor
     *
     * - Default sensor: SGP41
     * - User may override model type
     * - Example:
     *     SGP4x_7Semi sgp(SGP4x_7Semi::SGP40_SENSOR);
     */
    explicit SGP4x_7Semi(SGP4x model = SGPT41);


    /**
     * Initialize I2C interface
     *
     * - Sets I2C address
     * - Allows custom clock speed
     * - Allows custom SDA / SCL on ESP32
     * - Does not automatically verify sensor type
     */
    bool begin(uint8_t i2cAddress = 0x59,
               TwoWire &wirePort = Wire,
               uint32_t clockSpeed = 400000,
               int sda = -1,
               int scl = -1);


    /**
     * Perform soft reset
     *
     * - Sends general reset command
     * - Sensor requires re-conditioning after reset
     */
    bool softReset();


    /**
     * Return detected or configured sensor type
     *
     * - Returns SGP40_SENSOR or SGPT41
     */
    SGP4x readSensorType();


    /**
     * Read 48-bit serial number
     *
     * - Returned as 64-bit variable
     * - Upper bits remain zero
     */
    bool getSerialNumber(uint64_t &serialNumber);


    /**
     * Execute ONE conditioning cycle (SGP41 only)
     *
     * - Must be called 10 times
     * - Must be called once per second
     * - Required after power-up or heater-off
     */
    bool executeConditioning(float humidity_percent,
                             float temperature_c,
                             uint16_t &srawVoc);


    /**
     * Turn heater OFF
     *
     * - SGP41 requires full re-conditioning afterward
     */
    bool turnHeaterOff();


    /**
     * Read VOC raw signal
     *
     * - Automatically selects correct sensor routine
     */
    bool readVOC(uint16_t &voc);


    /**
     * Read VOC with environmental compensation
     *
     * - Uses humidity and temperature input
     * - Automatically selects correct sensor routine
     */
    bool readVOCComp(float humidity_percent,
                     float temperature_c,
                     uint16_t &voc);


    /**
     * SGP41 only
     *
     * - Read VOC and NOx raw signals
     */
    bool readRawSignalSGP41(uint16_t &voc,
                            uint16_t &nox);


    /**
     * SGP41 only
     *
     * - Read VOC and NOx raw signals
     * - With humidity and temperature compensation
     */
    bool readRawSignalSGP41Comp(float humidity_percent,
                                float temperature_c,
                                uint16_t &voc,
                                uint16_t &nox);


    /**
     * SGP40 only
     *
     * - Read raw VOC signal
     */
    bool readRawSignalSGP40(uint16_t &voc);


    /**
     * Get last measured raw VOC value
     */
    uint16_t getVOC();


    /**
     * Get last measured raw NOx value
     */
    uint16_t getNOx();


    /**
     * Get simplified VOC index
     *
     * - Linear placeholder calculation
     * - Not official Sensirion algorithm
     */
    float getVOCIndex();


    /**
     * Get simplified NOx index
     *
     * - Linear placeholder calculation
     * - Not official Sensirion algorithm
     */
    float getNOxIndex();


    /**
     * Get last recorded I2C error code
     */
    uint8_t getLastError() const;


    /**
     * Convert relative humidity to sensor ticks
     *
     * - Input range: 0 to 100 percent
     * - Output: 16-bit fixed point
     */
    static uint16_t humidityToTicks(float rh_percent);


    /**
     * Convert temperature to sensor ticks
     *
     * - Input range: -45 to +130 degrees Celsius
     * - Output: 16-bit fixed point
     */
    static uint16_t temperatureToTicks(float temperature_c);


private:

    /**
     * Internal I2C pointer
     */
    TwoWire *i2c = nullptr;

    /**
     * I2C address
     */
    uint8_t address = 0;

    /**
     * Initialization state flag
     */
    bool _initialized = false;

    /**
     * Configured sensor type
     */
    SGP4x sensor_type = SGPT41;

    /**
     * Cached raw values
     */
    uint16_t voc_raw = 0;
    uint16_t nox_raw = 0;

    /**
     * Last error code
     */
    uint8_t error = 0;


    /**
     * Low-level write helper
     *
     * - Sends command
     * - Optionally sends data words with CRC
     */
    bool writeCommand(uint16_t cmd,
                      const uint16_t *data = nullptr,
                      uint8_t words = 0);


    /**
     * Low-level read helper
     *
     * - Reads words
     * - Performs CRC validation
     */
    bool readWords(uint16_t *data,
                   uint8_t words);


    /**
     * CRC8 generator
     *
     * - Polynomial 0x31
     * - Initial value 0xFF
     */
    uint8_t generateCRC(const uint8_t *data,
                        uint8_t len);


    /**
     * Identify connected sensor
     *
     * - Reads feature set or response signature
     */
    SGP4x identifySensor();


    /**
     * Internal VOC index calculation
     *
     * - Simple linear approximation
     */
    float calculateVOCIndex(uint16_t raw);


    /**
     * Internal NOx index calculation
     *
     * - Simple linear approximation
     */
    float calculateNOxIndex(uint16_t raw);
};

#endif