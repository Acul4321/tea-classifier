#include <Arduino.h>
#include <../lib/BME688/Adafruit_BME680.h>
#include <../lib/SH41/7semi_SHT4x.h>
#include <../lib/SGP41/7Semi_SGP4x.h>
#include <../lib/AS7341/Adafruit_AS7341.h>

SHT4x_7semi SHT41;

SGP4x_7Semi SGP41;

Adafruit_AS7341 AS7341;

Adafruit_BME680 BME688(&Wire);
constexpr float SEA_LEVEL_PRESSURE_HPA = 1013.25f;
bool bme688ReadingPending = false;
uint32_t bme688ReadingEnd = 0;
uint32_t lastBme688Measurement = 0;

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

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  // init I2C bus
  Wire.begin(5, 6);
  delay(500);
  scanI2C();

  // ------------ BME688 sensor init ------------

  if (!BME688.begin(0x77)) {
    Serial.println("BME688 not detected");
    while (1);
  } 
  Serial.println("BME688 initialized successfully!");

  BME688.setTemperatureOversampling(BME680_OS_8X);
  BME688.setHumidityOversampling(BME680_OS_2X);
  BME688.setPressureOversampling(BME680_OS_4X);
  BME688.setIIRFilterSize(BME680_FILTER_SIZE_3);
  BME688.setGasHeater(320, 150);

  // ------------ SGP41 sensor init ------------
  // if (!SGP41.begin())
  // {
  //   Serial.println("Sensor not detected");
  //   while (1);
  // }

  // Serial.println("Starting SGP41 Conditioning...");
  // uint16_t voc, nox;
  // SGP41.readRawSignalSGP41(voc, nox);

  // for (int i = 0; i < 10; i++)
  // {
  //   SGP41.executeConditioning(50.0f, 25.0f, voc);
  //   delay(1000);
  // }

  // Serial.println("SGP41 Conditioning Complete");

  // ------------ SH41 sensor init ------------
  // if (SHT41.begin(Wire, 5, 6)) {
  //   Serial.println("SHT41 initialized successfully!");
  // } else {
  //   Serial.println("Failed to initialize SHT41.");
  //   while (1);
  // }
  // // setup measurement precision and heater settings
  // SHT41.setPrecision(REPEATABILITY_HIGH);
  // // SHT41.setHeater(HEATER_20mW_0_1s);

  // delay(2000); // wait for sensor to stabilize

  // ------------ AS7341 sensor init ------------
//   if (!AS7341.begin(0x39, &Wire)) {
//     Serial.println("AS7341 not detected");
//     while (1);
//   }
//   Serial.println("AS7341 initialized successfully!");

//   AS7341.setGain(AS7341_GAIN_16X);
//   AS7341.setATIME(29);
//   AS7341.setASTEP(599);

//   if (AS7341.setLEDCurrent(20) &&
//       AS7341.enableLED(true)) {
//     Serial.println("AS7341 LED enabled at 20 mA");
//   } else {
//     Serial.println("Failed to configure AS7341 LED");
//   }
}

void loop() {

  // ------------ BME688 sensor reading ------------
  if (bme688ReadingPending) {
    if (millis() >= bme688ReadingEnd) {
      if (BME688.endReading()) {
        Serial.println("BME688 readings:");
        Serial.print("Temperature: ");
        Serial.print(BME688.temperature, 2);
        Serial.println(" C");
        Serial.print("Pressure: ");
        Serial.print(BME688.pressure / 100.0f, 2);
        Serial.println(" hPa");
        Serial.print("Humidity: ");
        Serial.print(BME688.humidity, 2);
        Serial.println(" %RH");
        Serial.print("Gas resistance: ");
        Serial.print(BME688.gas_resistance / 1000.0f, 2);
        Serial.println(" kOhm");
        Serial.print("Approximate altitude: ");
        Serial.print(BME688.readAltitude(SEA_LEVEL_PRESSURE_HPA), 2);
        Serial.println(" m");
      } else {
        Serial.println("BME688: failed to complete reading");
      }
      bme688ReadingPending = false;
      lastBme688Measurement = millis();
    }
  } else if (millis() - lastBme688Measurement >= 2000) {
    bme688ReadingEnd = BME688.beginReading();
    if (bme688ReadingEnd != 0) {
      bme688ReadingPending = true;
    } else {
      Serial.println("BME688: failed to start reading");
      lastBme688Measurement = millis();
    }
  }
  

  // ------------ SGP41 sensor reading ------------
  // uint16_t voc, nox;

  // if (SGP41.readRawSignalSGP41Comp(50.0f, 25.0f, voc, nox)) {
  //   Serial.print("VOC Raw: ");
  //   Serial.print(voc);
  //   Serial.print(", NOx Raw: ");
  //   Serial.println(nox);
  // } else {
  //   Serial.println("SGP41: Failed to read VOC and NOx.");
  // }

  // ------------ SH41 sensor reading ------------
  // float temp, hum;

  // if (SHT41.readTemperatureHumidity(temp, hum)) {
  //   Serial.print("Temperature: ");
  //   Serial.print(temp, 2);
  //   Serial.print(" °C, Humidity: ");
  //   Serial.print(hum, 2);
  //   Serial.println(" %RH");
  // } else {
  //   Serial.println("SHT41: Failed to read temperature and humidity.");
  // }

  // ------------ AS7341 spectral reading ------------
  // uint16_t channels[12];
  
  // if (AS7341.readAllChannels(channels)) {
  //   Serial.println("AS7341 channels (raw counts):");
  //   Serial.print("F1 415nm: ");
  //   Serial.println(channels[0]);
  //   Serial.print("F2 445nm: ");
  //   Serial.println(channels[1]);
  //   Serial.print("F3 480nm: ");
  //   Serial.println(channels[2]);
  //   Serial.print("F4 515nm: ");
  //   Serial.println(channels[3]);
  //   Serial.print("F5 555nm: ");
  //   Serial.println(channels[6]);
  //   Serial.print("F6 590nm: ");
  //   Serial.println(channels[7]);
  //   Serial.print("F7 630nm: ");
  //   Serial.println(channels[8]);
  //   Serial.print("F8 680nm: ");
  //   Serial.println(channels[9]);
  //   Serial.print("Clear: ");
  //   Serial.println(channels[10]);
  //   Serial.print("NIR: ");
  //   Serial.println(channels[11]);
  // } else {
  //   Serial.println("AS7341: Failed to read spectral channels.");
  // }

  delay(1000);
}