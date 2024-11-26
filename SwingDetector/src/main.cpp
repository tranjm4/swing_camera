#include <Arduino.h>

#include <SparkFunLSM6DSO.h>
#include <Wire.h>

LSM6DSO imu;
#define CALIBRATION_TIME_MS 10000
#define READ_DELAY_MS 100
#define LED_INTERVAL 2000

#define BUTTON_PIN 33
#define LED_PIN 25

#define I2C_SLAVE_ADDR 0x42

// put function declarations here:
int myFunction(int, int);

float max_speed = INT_MIN;
float min_speed = INT_MAX;

bool button_pressed;
bool swing_detected;
int swing_count;
float threshold;

float get_speed(float dx, float dy, float dz)
{
  return sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

void calibrate_imu()
{
  /*
  Sets up min and max speed recorded 
  during a 10-second time window
  */
  u_long start_time = millis();
  u_long led_start_time = millis();
  bool led_on = true;

  while (millis() < start_time + CALIBRATION_TIME_MS) {
    float dx = imu.readFloatAccelX();
    float dy = imu.readFloatAccelY();
    float dz = imu.readFloatAccelZ();

    float speed = get_speed(dx, dy, dz);
    Serial.println(speed);

    if (speed > max_speed) {
      max_speed = speed;
    }
    if (speed < min_speed) {
      min_speed = speed;
    }

    if (millis() < led_start_time + LED_INTERVAL) {
      led_start_time = millis();
      if (led_on) {
        digitalWrite(LED_PIN, LOW);
        led_on = false;
      }
      else {
        digitalWrite(LED_PIN, HIGH);
        led_on = true;
      }
    }

    delay(READ_DELAY_MS);
  }

  digitalWrite(LED_PIN, LOW);

  Serial.print("Max speed: ");
  Serial.print(max_speed);
  Serial.print("\t");
  Serial.print("Min speed: ");
  Serial.println(min_speed);

  return;
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  Wire.begin(); // set up I2C

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  button_pressed = false;
  swing_detected = false;
  threshold = 0.35;
  swing_count = 0;

  if (imu.begin()) { // set up IMU initialization/calibration
    Serial.println("IMU connected");
    if (imu.initialize(BASIC_SETTINGS)) {
      Serial.println("Loaded Settings");
    }

    calibrate_imu();
  }
  else {
    Serial.println("Could not connect to IMU");
  }
}

void capture_image() {
  /*
  Captures an image on the camera module
  (current: Fujifilm X-T5)
  */
}

void loop() {
  if (digitalRead(BUTTON_PIN) == 0) {
    if (button_pressed) {
      button_pressed = false;
      digitalWrite(LED_PIN, LOW);
    }
    else {
      button_pressed = true;
      digitalWrite(LED_PIN, HIGH);
    }
  }
  float dx = imu.readFloatAccelX();
  float dy = imu.readFloatAccelY();
  float dz = imu.readFloatAccelZ();

  float speed = get_speed(dx, dy, dz);

  float mapped_speed = (speed - min_speed) / (max_speed - min_speed);
  if (mapped_speed > 1) {
    mapped_speed = 1;
  }
  else if (mapped_speed < 0) {
    mapped_speed = 0;
  }

  if (!swing_detected && mapped_speed > threshold) {
    capture_image();
    swing_detected = true;
    swing_count ++;

    Serial.print("Swing detected! Total: ");
    Serial.print(swing_count);
    Serial.print("\t\t");
    Serial.print("Button pressed: ");
    Serial.println(button_pressed);

    // send high signal to camera ESP
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(0x01);
    Wire.endTransmission();
  }
  if (mapped_speed < threshold) {
    swing_detected = false;
  }

  delay(READ_DELAY_MS);
}