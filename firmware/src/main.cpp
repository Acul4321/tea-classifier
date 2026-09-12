#include <Arduino.h>
#include <../lib/SH41/7semi_SHT4x.h>
#include <../lib/SGP41/7Semi_SGP4x.h>

SHT4x_7semi SHT41;
SGP4x_7Semi SGP41;

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

  // ------------ SGP41 sensor init ------------
  if (!SGP41.begin())
  {
    Serial.println("Sensor not detected");
    while (1);
  }

  Serial.println("Starting SGP41 Conditioning...");
  uint16_t voc, nox;
  SGP41.readRawSignalSGP41(voc, nox);

  for (int i = 0; i < 10; i++)
  {
    SGP41.executeConditioning(50.0f, 25.0f, voc);
    delay(1000);
  }

  Serial.println("SGP41 Conditioning Complete");

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
}

void loop() {

  // ------------ SGP41 sensor reading ------------
  uint16_t voc, nox;

  if (SGP41.readRawSignalSGP41Comp(50.0f, 25.0f, voc, nox)) {
    Serial.print("VOC Raw: ");
    Serial.print(voc);
    Serial.print(", NOx Raw: ");
    Serial.println(nox);
  } else {
    Serial.println("SGP41: Failed to read VOC and NOx.");
  }

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

  delay(1000);
}