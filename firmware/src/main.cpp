#include <Arduino.h>
#include <../lib/BME688/Adafruit_BME680.h>
#include <../lib/SH41/7semi_SHT4x.h>
#include <../lib/SGP41/7Semi_SGP4x.h>
#include <../lib/AS7341/Adafruit_AS7341.h>
#include <../lib/CJMCU-6814/CJMCU6814.h>
#include "sensors/Sensors.h"

SHT4x_7semi SHT41;
SGP4x_7Semi SGP41;
Adafruit_AS7341 AS7341;
Adafruit_BME680 BME688(&Wire);
CJMCU6814 MCU6814;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    sensor_init();
    scanI2C();
}

void loop() {
    readAllSensors();
    delay(1000);
}
