/*
 * Demo sketch for Adafruit Feather Adalogger
 * Original code by Adafruit
 * found here: https://gist.githubusercontent.com/ladyada/13efab4022b7358033c7/raw/8387409d8f9b1c2157bf8c9e78dff3c0a3b0007d/adalogger.ino
 * 
 * Also includes code from MMA8451demo, also from Adafruit
 * 
 * Modified by SC for simple battery voltage reading and accelerometer values logged to SD card
 * 22-Dec-16
 */


#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
//#include <Adafruit_Sensor.h>

#define BMP_SCK 13 //SPI Clock（時刻の同期）
#define BMP_MISO 12 //SPI Master Input Slave Output（周辺機器側から制御側へデータを送信）
#define BMP_MOSI 11 //SPI Master Output Slave Input（制御側から周辺機器側へデータを送信）
#define BMP_CS 10 // SPI Chip Select（デバイスを選択する信号）

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

// Set the pins used
#define cardSelect 4

// Define the input for battery voltage reading
#define VBATPIN A7

File logfile;

// Blink out an error code
void error(uint8_t errno) {
  while (1) {
    for (uint8_t i = 0; i < errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (uint8_t i = errno; i < 10; i++) {
      delay(200);
    }
  }
}

void setup() {
  Serial.begin(9600);

  while (!Serial) delay(100); // Wait for native USB
  Serial.println(F("BMP280 Sensor event test"));
  pinMode(13, OUTPUT);

  // BMP280 initialization
  unsigned status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    while (1) delay(10);
  }

  // Modify BMP280 settings
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,        // Operating Mode
    Adafruit_BMP280::SAMPLING_X1,        // Temp. oversampling (Default_x1)
    Adafruit_BMP280::SAMPLING_X4,        // Pressure oversampling (x4_--Default_x16)
    Adafruit_BMP280::FILTER_OFF,         // Filter disabled
    Adafruit_BMP280::STANDBY_MS_1        // Standby time (1ms)
  );

  bmp_temp->printSensorDetails();

  // Initialize SD card
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "ALTLOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i / 10;
    filename[7] = '0' + i % 10;
    if (!SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.print("Couldn't create ");
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to ");
  Serial.println(filename);

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.println("Ready!");
}

void loop() {
  digitalWrite(8, HIGH);

  // Battery voltage reading and logging
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;      // Scale to actual voltage
  measuredvbat *= 3.3;    // Reference voltage
  measuredvbat /= 1024;   // Convert to voltage
  logfile.print("VBat = "); logfile.println(measuredvbat);
  Serial.print("Voltage of Battery: "); Serial.println(measuredvbat);

  // Read temperature and pressure events
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  // Use memcpy to safely extract data
  float temperature, pressure;
  memcpy(&temperature, &temp_event.temperature, sizeof(float));
  memcpy(&pressure, &pressure_event.pressure, sizeof(float));

  Serial.print(F("Temperature = "));
  Serial.print(temperature);
  Serial.println(" *C");
  logfile.print(F("Temperature = "));
  logfile.print(temperature);
  logfile.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure);
  Serial.println(" hPa");
  logfile.print(F("Pressure = "));
  logfile.print(pressure);
  logfile.println(" hPa");

  logfile.println();
  logfile.flush();

  digitalWrite(8, LOW);

  // Adjust interval between each reading (200ms)
  delay(200);
}
