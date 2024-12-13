#ifndef SCREEN_H
#define SCREEN_H

typedef void (*screen_off_cb_t)(void);

#include "screen.h"

#include <U8g2lib.h>
#include <Wire.h>
#include "esp_camera.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2 = NULL;
static screen_off_cb_t off_cb = NULL;

void setupScreen(bool camera)
{
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0)
    {
        Serial.println("Started OLED");
        u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R2, U8X8_PIN_NONE);
        u8g2->begin();
        u8g2->clearBuffer();
        u8g2->setFlipMode(0);
        u8g2->setFontMode(1); // Transparent
        u8g2->setDrawColor(1);
        u8g2->setFontDirection(0);
        u8g2->firstPage();
        do
        {
            u8g2->setFont(u8g2_font_inb19_mr);
            u8g2->drawStr(0, 30, "LilyGo");
            u8g2->drawHLine(2, 35, 47);
            u8g2->drawHLine(3, 36, 47);
            u8g2->drawVLine(45, 32, 12);
            u8g2->drawVLine(46, 33, 12);
            u8g2->setFont(u8g2_font_inb19_mf);
            u8g2->drawStr(58, 60, "Cam");
        } while (u8g2->nextPage());

        u8g2->setFont(u8g2_font_fur11_tf);
        if (camera)
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s)
            {
                camera_sensor_info_t *sinfo = esp_camera_sensor_get_info(&(s->id));
                u8g2->drawStr(0, 58, sinfo->name);
            }
        }
        else
        {
            u8g2->drawStr(0, 58, "N/A");
        }
        u8g2->sendBuffer();
        delay(5000);
    }
}

#endif