#include <Arduino.h>

#include <SparkFunLSM6DSO.h>
#include <Wire.h>

#include "nvs.h"
#include "nvs_flash.h"

#include <HTTPClient.h>
#include <WiFi.h>

// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#include <Base64.h>
#include <stdlib.h>

LSM6DSO imu;

#define CALIBRATION_TIME_MS 5000
#define READ_DELAY_MS 100
#define LED_INTERVAL 2000

#define BUTTON_PIN 33
#define LED_PIN 25

#define SDA_1 21
#define SCL_1 22

#define SDA_2 15
#define SCL_2 13

#define I2C_SLAVE_ADDR 0x42
TwoWire I2C_BUS_2 = TwoWire(1);

#define AWS_IP_ADDRESS "http://18.218.247.182:5000"

int delayMS;

// This example downloads the URL "http://arduino.cc/"
char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = AWS_IP_ADDRESS;
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("Done\n");
      // Serial.printf("SSID = %s\n", ssid);
      // Serial.printf("PASSWD = %s\n", pass);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

// BLECharacteristic *pCharacteristic;

void setup_wifi();
// void setup_bluetooth();

// class MyCallbacks : public BLECharacteristicCallbacks
// {
//   void onWrite(BLECharacteristic *pCharacteristic)
//   {
//     std::string value = pCharacteristic->getValue();
//     if (value.length() > 0)
//     {
//       Serial.println("*********");
//       Serial.print("New value: ");
//       for (int i = 0; i < value.length(); i++)
//         Serial.print(value[i]);
//       Serial.println();
//       Serial.println("*********");

//       if (value == "on")
//       {
//         digitalWrite(LED_PIN, HIGH);
//       }
//       else if (value == "off")
//       {
//         digitalWrite(LED_PIN, LOW);
//       }
//     }
//   }
// };

// put function declarations here:
int myFunction(int, int);

float max_speed = INT_MIN;
float min_speed = INT_MAX;

bool button_pressed;
bool swing_detected;
int swing_count;
float threshold;

#define IMAGE_SIZE 50
void send_signal_to_camera();
void get_image();
void get_image2();
void send_image_bluetooth();
void send_image_wifi();

// uint8_t image[IMAGE_SIZE][IMAGE_SIZE];
byte image[IMAGE_SIZE][IMAGE_SIZE];

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

  while (millis() < start_time + CALIBRATION_TIME_MS)
  {
    float dx = imu.readFloatAccelX();
    float dy = imu.readFloatAccelY();
    float dz = imu.readFloatAccelZ();

    float speed = get_speed(dx, dy, dz);
    Serial.println(speed);

    if (speed > max_speed)
    {
      max_speed = speed;
    }
    if (speed < min_speed)
    {
      min_speed = speed;
    }

    if (millis() < led_start_time + LED_INTERVAL)
    {
      led_start_time = millis();
      if (led_on)
      {
        digitalWrite(LED_PIN, LOW);
        led_on = false;
      }
      else
      {
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

void setup()
{
  Serial.begin(9600);
  delay(2000);
  Wire.begin(); // set up I2C to read IMU data

  I2C_BUS_2.begin(SDA_2, SCL_2); // set up I2C to send data to Camera Module

  // setup_bluetooth();

  nvs_access();
  delay(1000);
  setup_wifi();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  button_pressed = false;
  swing_detected = false;
  threshold = 0.35;
  swing_count = 0;

  if (imu.begin())
  { // set up IMU initialization/calibration
    Serial.println("IMU connected");
    if (imu.initialize(BASIC_SETTINGS))
    {
      Serial.println("Loaded Settings");
    }

    calibrate_imu();
  }
  else
  {
    Serial.println("Could not connect to IMU");
  }
}

void loop()
{
  if (digitalRead(BUTTON_PIN) == 0)
  {
    if (button_pressed)
    {
      button_pressed = false;
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      button_pressed = true;
      digitalWrite(LED_PIN, HIGH);
    }
  }
  float dx = imu.readFloatAccelX();
  float dy = imu.readFloatAccelY();
  float dz = imu.readFloatAccelZ();

  float speed = get_speed(dx, dy, dz);

  float mapped_speed = (speed - min_speed) / (max_speed - min_speed);
  if (mapped_speed > 1)
  {
    mapped_speed = 1;
  }
  else if (mapped_speed < 0)
  {
    mapped_speed = 0;
  }

  if (!swing_detected && mapped_speed > threshold)
  {
    // capture_image();
    swing_detected = true;
    swing_count++;
    Serial.print("Swing detected! Total: ");
    Serial.print(swing_count);
    Serial.print("\t\t");
    Serial.print("Button pressed: ");
    Serial.println(button_pressed);

    // send high signal to camera ESP
    send_signal_to_camera();
    delay(1000);
    // get_image();
    get_image2();

    // send image data to bluetooth
    // send_image_bluetooth();
    send_image_wifi();
  }
  if (mapped_speed < threshold)
  {
    swing_detected = false;
  }

  delay(READ_DELAY_MS);
}

void send_signal_to_camera()
{
  I2C_BUS_2.beginTransmission(I2C_SLAVE_ADDR);
  I2C_BUS_2.write(0x01);
  I2C_BUS_2.endTransmission();
}

// void get_image()
// {
//   for (int i = 0; i < IMAGE_SIZE; i++)
//   {
//     I2C_BUS_2.requestFrom(I2C_SLAVE_ADDR, IMAGE_SIZE);
//     int index = 0;
//     while (I2C_BUS_2.available())
//     {
//       uint8_t value = I2C_BUS_2.read();
//       if (value == 0)
//         image[i][index++] = I2C_BUS_2.read();
//     }
//     Serial.print("Received row: ");
//     Serial.println(i);
//     Serial.print("\t");
//     for (int j = 0; j < IMAGE_SIZE; j++)
//     {
//       Serial.print(image[i][j]);
//       Serial.print(", ");
//     }
//     Serial.println("");

//     delay(100);
//   }
// }

void get_image2()
{
  for (int i = 0; i < IMAGE_SIZE; i++)
  {
    for (int j = 0; j < IMAGE_SIZE; j++)
    {
      image[i][j] = random(0, 256);
    }
  }
}

// void send_image_bluetooth()
// {
//   // for (int i = 0; i < IMAGE_SIZE * IMAGE_SIZE; i++)
//   // {
//   //   int chunk_size = min(20, IMAGE_SIZE*IMAGE_SIZE - i);
//   //   pCharacteristic->setValue(&image[i], 1);
//   //   pCharacteristic->notify();
//   //   delay(10);
//   // }
//   for (int i = 0; i < IMAGE_SIZE * IMAGE_SIZE; i += 600)
//   {
//     size_t size = min(600, IMAGE_SIZE * IMAGE_SIZE - i);
//     pCharacteristic->setValue(image, size);
//     pCharacteristic->notify();
//   }
// }



String encode_image() {
  uint8_t *raw_data = &image[0][0];
  size_t data_size = IMAGE_SIZE * IMAGE_SIZE;

  String encoded_image = base64::encode(raw_data, data_size);
  return encoded_image;
}

void send_image_wifi()
{
  // sends a base64-encoded image
  String encoded_image = encode_image();

  WiFiClient client;
  HTTPClient http;

  String server_route = AWS_IP_ADDRESS;
  server_route += "/upload";
  String payload = "{\"imageData\":\"" + encoded_image + "\"}";
  Serial.println("PAYLOAD:");
  Serial.println(payload);

  http.begin(server_route);
  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  // server_route += "/";

  // Serial.println(server_route);

  // http.begin(client, server_route.c_str());
  // int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString(); // Get the response string from the server
    Serial.println("Server response: ");
    Serial.println(response);
  }
  else
  {
    Serial.println("Error in sending POST request");
    Serial.print("HTTP error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// void setup_bluetooth()
// {
//   delay(3000);
//   BLEDevice::init("CS147_77");
//   BLEServer *pServer = BLEDevice::createServer();
//   BLEService *pService = pServer->createService(SERVICE_UUID);
//   pCharacteristic = pService->createCharacteristic(
//       CHARACTERISTIC_UUID,
//       BLECharacteristic::PROPERTY_READ |
//           BLECharacteristic::PROPERTY_WRITE);
//   pCharacteristic->setCallbacks(new MyCallbacks());
//   pCharacteristic->setValue("Hello World");

//   pService->start();
//   BLEAdvertising *pAdvertising = pServer->getAdvertising();
//   pAdvertising->start();

//   Serial.println("Bluetooth configured");
// }

void setup_wifi()
{
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.print("\t");
    Serial.println(pass);
    Serial.println(".");
  }
}