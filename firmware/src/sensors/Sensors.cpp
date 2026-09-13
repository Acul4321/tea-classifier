#include "Sensors.h"

#include <../lib/BME688/Adafruit_BME680.h>
#include <../lib/SH41/7semi_SHT4x.h>
#include <../lib/SGP41/7Semi_SGP4x.h>
#include <../lib/AS7341/Adafruit_AS7341.h>
#include <../lib/CJMCU-6814/CJMCU6814.h>

extern SHT4x_7semi SHT41;
extern SGP4x_7Semi SGP41;
extern Adafruit_AS7341 AS7341;
extern Adafruit_BME680 BME688;
extern CJMCU6814 MCU6814;

namespace {
constexpr int I2C_SDA_PIN = 5;
constexpr int I2C_SCL_PIN = 6;
constexpr uint32_t CJMCU_WARMUP_TIME_MS = 10UL * 60UL * 1000UL;
constexpr float SEA_LEVEL_PRESSURE_HPA = 1013.25f;
uint32_t cjmcuWarmupStarted = 0;
}

// helper function to scan devices on the I2C bus
void scanI2C() {
  Serial.println("Scanning I2C...");
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
}

bool cjmcuReady() {
  return cjmcuWarmupStarted != 0 && millis() - cjmcuWarmupStarted >= CJMCU_WARMUP_TIME_MS;
}

void initBME688() {
  if (!BME688.begin(0x77)) {
    Serial.println("BME688 not detected");
  } else {
    BME688.setTemperatureOversampling(BME680_OS_8X);
    BME688.setHumidityOversampling(BME680_OS_2X);
    BME688.setPressureOversampling(BME680_OS_4X);
    BME688.setIIRFilterSize(BME680_FILTER_SIZE_3);
    BME688.setGasHeater(320, 150);
    Serial.println("BME688 initialized");
  }
}

void initSGP41() {
  if (!SGP41.begin(0x59, Wire, 400000, I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("SGP41 not detected");
  } else {
    Serial.println("SGP41 conditioning started (10 seconds)");
    uint16_t srawVoc = 0;
    for (uint8_t cycle = 0; cycle < 10; ++cycle) {
      if (!SGP41.executeConditioning(50.0f, 25.0f, srawVoc)) {
        Serial.println("SGP41 conditioning failed");
        break;
      }
      delay(1000);
    }
    Serial.println("SGP41 conditioning complete");
  }
}

void initSHT41() {
  if (!SHT41.begin(Wire, I2C_SDA_PIN, I2C_SCL_PIN)) {
    Serial.println("SHT41 not detected");
  } else {
    SHT41.setPrecision(REPEATABILITY_HIGH);
    Serial.println("SHT41 initialized");
  }
}

void initAS7341() {
  if (!AS7341.begin(0x39, &Wire)) {
    Serial.println("AS7341 not detected");
  } else {
    AS7341.setGain(AS7341_GAIN_16X);
    AS7341.setATIME(29);
    AS7341.setASTEP(599);
    AS7341.setLEDCurrent(20);
    AS7341.enableLED(true);
    Serial.println("AS7341 initialized");
  }
}

void initCJMCU6814() {
  MCU6814.begin();
  cjmcuWarmupStarted = millis();
  Serial.println("CJMCU-6814 initialized; allow 10 minutes for warm-up");
}

void sensor_init() {
  // init I2C bus
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  initBME688();
  initSGP41();
  initSHT41();
  initAS7341();
  initCJMCU6814();
}

BME688Data readBME688() {
  BME688Data data;
  data.valid = BME688.performReading();
  if (data.valid) {
    data.temperature = BME688.temperature;
    data.pressure = BME688.pressure / 100.0f;
    data.humidity = BME688.humidity;
    data.gasResistance = BME688.gas_resistance / 1000.0f;
    data.altitude = BME688.readAltitude(SEA_LEVEL_PRESSURE_HPA);
  }
  Serial.printf("BME688: %s, T=%.2f C, P=%.2f hPa, RH=%.2f %%, Gas=%.2f kOhm, Alt=%.2f m\n",
                data.valid ? "OK" : "FAILED", data.temperature, data.pressure,
                data.humidity, data.gasResistance, data.altitude);
  return data;
}

SHT41Data readSHT41() {
  SHT41Data data;
  data.valid = SHT41.readTemperatureHumidity(data.temperature, data.humidity);
  Serial.printf("SHT41: %s, T=%.2f C, RH=%.2f %%\n", data.valid ? "OK" : "FAILED",
                data.temperature, data.humidity);
  return data;
}

SGP41Data readSGP41() {
  SGP41Data data;
  data.valid = SGP41.readRawSignalSGP41Comp(50.0f, 25.0f, data.vocRaw, data.noxRaw);
  Serial.printf("SGP41: %s, VOC raw=%u, NOx raw=%u\n", data.valid ? "OK" : "FAILED",
                data.vocRaw, data.noxRaw);
  return data;
}

AS7341Data readAS7341() {
  AS7341Data data;
  data.valid = AS7341.readAllChannels(data.channels);
  Serial.printf("AS7341: %s, F1=%u, F2=%u, F3=%u, F4=%u, F5=%u, F6=%u, F7=%u, F8=%u, Clear=%u, NIR=%u\n",
                data.valid ? "OK" : "FAILED", data.channels[0], data.channels[1],
                data.channels[2], data.channels[3], data.channels[6], data.channels[7],
                data.channels[8], data.channels[9], data.channels[10], data.channels[11]);
  return data;
}

CJMCU6814Data readCJMCU6814() {
  CJMCU6814Data data;
  data.ready = cjmcuReady();
  data.valid = data.ready;
  if (data.ready) {
    MCU6814.readGases(data.co, data.nh3, data.no2);
  }
  Serial.printf("CJMCU-6814: %s, CO=%.2f ppm, NH3=%.2f ppm, NO2=%.2f ppm\n",
                data.ready ? "OK" : "WARMING UP", data.co, data.nh3, data.no2);
  return data;
}

AllSensorData readAllSensors() {
  AllSensorData data;
  data.bme688 = readBME688();
  data.sht41 = readSHT41();
  data.sgp41 = readSGP41();
  data.as7341 = readAS7341();
  data.cjmcu6814 = readCJMCU6814();
  return data;
}
