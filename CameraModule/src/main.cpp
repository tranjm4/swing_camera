#include <Arduino.h>
#include <Wire.h>
#include "esp_camera.h"

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

// #include <HttpClient.h>
// #include <WiFi.h>
// #include <inttypes.h>
// #include <stdio.h>
// #include "esp_system.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "nvs.h"
// #include "nvs_flash.h"

#define SIOC_PIN 4
#define SIOD_PIN 5
#define HREF_PIN 18
#define PCLK_PIN 12
#define XCLK_PIN 38
#define VSYNC_PIN 8
#define RESET_PIN 39
#define PWDN_PIN -1

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

void initPowerChip();
void configCamera();
void initCamera();
sensor_t *captureImage();
void displayImage();

void receiveEvent(int numBytes)
{
  while (Wire.available())
  {
    byte signal = Wire.read();
    if (signal == 0x01)
    {
      highSignalReceived = true;
    }
  }
}

camera_config_t config;
XPowersPMU PMU;

sensor_t *image;

void setup()
{
  // put your setup code here, to run once:
  // configure custom SDA, SCL pins
  // Wire.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  // Wire.onReceive(receiveEvent);
  Serial.begin(9600);

  configCamera();

  initCamera();

  image = captureImage();  
}

void loop()
{
  // put your main code here, to run repeatedly:
  // if (highSignalReceived) {
  //   Serial.println("High signal received!");
  //   highSignalReceived = false;

  //   captureImage();
  // }
  Serial.println("Hi");

  // display image
  displayImage();
  delay(10000);
}

void initPowerChip()
{
  // turns on camera power channel
  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
  {
    Serial.println("Failed to initialize power.....");
    delay(5000);
  }

  // set working voltage of camera
  PMU.setALDO1Voltage(1800);
  PMU.enableALDO1();
  PMU.setALDO2Voltage(2800);
  PMU.enableALDO2();
  PMU.setALDO4Voltage(3000);
  PMU.enableALDO4();

  PMU.disableTSPinMeasure();
}

void configCamera()
{
  // CODE OBTAINED FROM EXAMPLE CODE: https://github.com/Xinyuan-LilyGO/LilyGo-Cam-ESP32S3/blob/master/examples/MinimalCameraExample/MinimalCameraExample.ino
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = DATA2_PIN;
  config.pin_d1 = DATA3_PIN;
  config.pin_d2 = DATA4_PIN;
  config.pin_d3 = DATA5_PIN;
  config.pin_d4 = DATA6_PIN;
  config.pin_d5 = DATA7_PIN;
  config.pin_d6 = DATA8_PIN;
  config.pin_d7 = DATA9_PIN;
  config.pin_xclk = XCLK_PIN;
  config.pin_pclk = PCLK_PIN;
  config.pin_vsync = VSYNC_PIN;
  config.pin_href = HREF_PIN;
  config.pin_sscb_sda = SIOD_PIN;
  config.pin_sscb_scl = SIOC_PIN;
  config.pin_pwdn = PWDN_PIN;
  config.pin_reset = RESET_PIN;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    if (psramFound())
    {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }
  else
  {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
  }
}

void initCamera()
{
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x Please check if the camera is connected well.", err);
    while (1)
    {
      delay(5000);
    }
  }
}

sensor_t *captureImage()
{
  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1);       // flip it back
    s->set_brightness(s, 1);  // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  return s;
}

void displayImage() {
  
}