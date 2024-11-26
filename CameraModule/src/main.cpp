#include <Arduino.h>
#include <Wire.h>

#define SIOC_PIN 4
#define SIOD_PIN 5
#define HREF_PIN 18
#define PCLK_PIN 12
#define XCLK_PIN 38
#define VSYNC_PIN 8
#define RESET_PIN 39

#define DATA2_PIN 14
#define DATA3_PIN 47
#define DATA4_PIN 48
#define DATA5_PIN 21
#define DATA6_PIN 13
#define DATA7_PIN 11
#define DATA8_PIN 10
#define DATA9_PIN 9

#define SDA_PIN 45
#define SCL_PIN 3

#define I2C_SLAVE_ADDR 0x42
volatile bool highSignalReceived = false;

void captureImage();

void receiveEvent(int numBytes) {
  while (Wire.available()) {
    byte signal = Wire.read();
    if (signal == 0x01) {
      highSignalReceived = true;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  // configure custom SDA, SCL pins
  Wire.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (highSignalReceived) {
    Serial.println("High signal received!");
    highSignalReceived = false;

    captureImage();
  }
}

void captureImage() {
  return;
}