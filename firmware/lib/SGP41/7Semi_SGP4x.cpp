#include "7Semi_SGP4x.h"



/**
 * Constructor
 *
 * - Sensor type fixed at creation
 * - No auto-detection used
 */
SGP4x_7Semi::SGP4x_7Semi(SGP4x model)
    : sensor_type(model)
{
}


bool SGP4x_7Semi::begin(uint8_t i2cAddress,
                        TwoWire &wirePort,
                        uint32_t clockSpeed,
                        int sda,
                        int scl)
{
    i2c = &wirePort;
    address = i2cAddress;

#ifdef ESP32
    if (sda != -1 && scl != -1)
        i2c->begin(sda, scl);
    else
        i2c->begin();
#else
    (void)sda;
    (void)scl;
    i2c->begin();
#endif

    i2c->setClock(clockSpeed);
    delay(2);

    _initialized = true;
    return true;
}

SGP4x_7Semi::SGP4x SGP4x_7Semi::readSensorType() 
{
    return sensor_type;
}
/**
 * Execute ONE SGP41 conditioning cycle
 */
bool SGP4x_7Semi::executeConditioning(float humidity_percent,
                                      float temperature_c,
                                      uint16_t &srawVoc)
{
    if (!_initialized || sensor_type != SGPT41)
        return false;

    uint16_t params[2] =
    {
        humidityToTicks(humidity_percent),
        temperatureToTicks(temperature_c)
    };

    if (!writeCommand(CMD_SGP41_CONDITIONING, params, 2))
        return false;

    delay(SGP41_DELAY_MS);

    return readWords(&srawVoc, 1);
}

/**
 * Read VOC (auto switch SGP40 / SGP41)
 */
bool SGP4x_7Semi::readVOC(uint16_t &voc)
{
    switch (sensor_type)
    {
        case SGP40_SENSOR:
            return readRawSignalSGP40(voc);

        case SGPT41:
        {
            uint16_t nox_dummy;
            return readRawSignalSGP41(voc, nox_dummy);
        }

        default:
            return false;
    }
}

/**
 * Read VOC with compensation (auto switch)
 */
bool SGP4x_7Semi::readVOCComp(float humidity_percent,
                              float temperature_c,
                              uint16_t &voc)
{
    switch (sensor_type)
    {
        case SGP40_SENSOR:
        {
            return readRawSignalSGP40(voc);
        }

        case SGPT41:
        {
            uint16_t nox_dummy;
            return readRawSignalSGP41Comp(humidity_percent,
                                          temperature_c,
                                          voc,
                                          nox_dummy);
        }

        default:
            return false;
    }
}

bool SGP4x_7Semi::readRawSignalSGP41(uint16_t &voc,
                                     uint16_t &nox)
{
    return readRawSignalSGP41Comp(50.0f, 25.0f, voc, nox);
}

bool SGP4x_7Semi::readRawSignalSGP41Comp(float humidity_percent,
                                         float temperature_c,
                                         uint16_t &voc,
                                         uint16_t &nox)
{
    if (!_initialized || sensor_type != SGPT41)
        return false;

    uint16_t params[2] =
    {
        humidityToTicks(humidity_percent),
        temperatureToTicks(temperature_c)
    };

    if (!writeCommand(CMD_SGP41_MEASURE_RAW, params, 2))
        return false;

    delay(SGP41_DELAY_MS);

    uint16_t buffer[2];
    if (!readWords(buffer, 2))
        return false;

    voc_raw = buffer[0];
    nox_raw = buffer[1];

    voc = voc_raw;
    nox = nox_raw;

    return true;
}

bool SGP4x_7Semi::readRawSignalSGP40(uint16_t &voc)
{
    if (!_initialized || sensor_type != SGP40_SENSOR)
        return false;

    const uint16_t params[2] =
    {
        default_humidity_ticks,
        default_temperature_ticks
    };

    if (!writeCommand(CMD_SGP40_MEASURE_RAW, params, 2))
        return false;

    delay(SGP40_DELAY_MS);

    if (!readWords(&voc_raw, 1))
        return false;

    voc = voc_raw;
    return true;
}

/** Low level I2C helpers */
bool SGP4x_7Semi::writeCommand(uint16_t cmd,
                               const uint16_t *data,
                               uint8_t words)
{
    i2c->beginTransmission(address);
    i2c->write(cmd >> 8);
    i2c->write(cmd & 0xFF);

    for (uint8_t i = 0; i < words; i++)
    {
        uint8_t buf[2] =
        {
            (uint8_t)(data[i] >> 8),
            (uint8_t)(data[i] & 0xFF)
        };

        i2c->write(buf, 2);
        i2c->write(generateCRC(buf, 2));
    }

    return (i2c->endTransmission() == 0);
}

bool SGP4x_7Semi::readWords(uint16_t *data,
                            uint8_t words)
{
    uint8_t expected = words * 3;
    if (i2c->requestFrom(address, expected) != expected)
        return false;

    for (uint8_t i = 0; i < words; i++)
    {
        uint8_t msb = i2c->read();
        uint8_t lsb = i2c->read();
        uint8_t crc = i2c->read();

        uint8_t buf[2] = {msb, lsb};
        if (generateCRC(buf, 2) != crc)
            return false;

        data[i] = (msb << 8) | lsb;
    }

    return true;
}

uint8_t SGP4x_7Semi::generateCRC(const uint8_t *data,
                                 uint8_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }

    return crc;
}

float SGP4x_7Semi::calculateVOCIndex(uint16_t raw)
{
    return (raw > 20000) ? (raw - 20000) / 100.0f : 0.0f;
}

float SGP4x_7Semi::calculateNOxIndex(uint16_t raw)
{
    return (raw > 10000) ? (raw - 10000) / 200.0f : 0.0f;
}

uint16_t SGP4x_7Semi::humidityToTicks(float rh)
{
    if (rh < 0) rh = 0;
    if (rh > 100) rh = 100;
    return (uint16_t)((rh * 65535.0f / 100.0f) + 0.5f);
}

uint16_t SGP4x_7Semi::temperatureToTicks(float t)
{
    if (t < -45) t = -45;
    if (t > 130) t = 130;
    return (uint16_t)(((t + 45.0f) * 65535.0f / 175.0f) + 0.5f);
}