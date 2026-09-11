#include <Arduino.h>
#include <../lib/SH41/7semi_SHT4x.h>

SHT4x_7semi SH41_sensor;


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
  Wire.begin(5, 6);
  delay(500);
  scanI2C();

  // init SH41 sensor
  if (SH41_sensor.begin(Wire, 5, 6)) {
    Serial.println("SHT41 initialized successfully!");
  } else {
    Serial.println("Failed to initialize SHT41.");
    while (1);
  }

  // setup measurement precision and heater settings
  SH41_sensor.setPrecision(REPEATABILITY_HIGH);
  // SH41_sensor.setHeater(HEATER_20mW_0_1s);

  delay(2000); // wait for sensor to stabilize
}

void loop() {
  float temp, hum;

  if (SH41_sensor.readTemperatureHumidity(temp, hum)) {
    Serial.print("Temperature: ");
    Serial.print(temp, 2);
    Serial.print(" °C, Humidity: ");
    Serial.print(hum, 2);
    Serial.println(" %RH");
  } else {
    Serial.println("Failed to read temperature and humidity.");
  }

  delay(1000);
}