
#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__

#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "esp_system.h"

#include "string.h"

#include "i2c_oled_driver.h"
// #include "spi_oled_driver.h"

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

#define _max(a,b) ((a) > (b) ? (a) : (b))
#define _min(a,b) ((a) < (b) ? (a) : (b))


#define DEBUG_OLEDDISPLAY printf

enum OLEDDISPLAY_COLOR {
  BLACK = 0,
  WHITE = 1,
  INVERSE = 2
};
enum OLED_Font_Size
{
  Plain_10 = 10,
  Plain_16 = 16,
  Plain_24 = 24
};

enum OLEDDISPLAY_TEXT_ALIGNMENT {
  TEXT_ALIGN_LEFT = 0,           // 输入坐标为左起点
  TEXT_ALIGN_RIGHT = 1,          // 输入坐标为右起点
  TEXT_ALIGN_CENTER = 2,         // 输入坐标为水平中心
  TEXT_ALIGN_CENTER_BOTH = 3     // 输入坐标为整个字符串的水平和垂直中心
};
extern uint8_t textAlignment ;
extern const uint16_t  displayWidth;
extern const uint16_t  displayHeight;

// Display commands
#define CHARGEPUMP 0x8D
#define COLUMNADDR 0x21
#define COMSCANDEC 0xC8
#define COMSCANINC 0xC0
#define DISPLAYALLON 0xA5
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define EXTERNALVCC 0x1
#define INVERTDISPLAY 0xA7
#define MEMORYMODE 0x20
#define NORMALDISPLAY 0xA6
#define PAGEADDR 0x22
#define SEGREMAP 0xA0
#define SETCOMPINS 0xDA
#define SETCONTRAST 0x81
#define SETDISPLAYCLOCKDIV 0xD5
#define SETDISPLAYOFFSET 0xD3
#define SETHIGHCOLUMN 0x10
#define SETLOWCOLUMN 0x00
#define SETMULTIPLEX 0xA8
#define SETPRECHARGE 0xD9
#define SETSEGMENTREMAP 0xA1
#define SETSTARTLINE 0x40
#define SETVCOMDETECT 0xDB
#define SWITCHCAPVCC 0x2



  esp_err_t oled_init();

  esp_err_t oled_set_pos(uint8_t x_start, uint8_t y_start);

  esp_err_t oled_Fully_fill(uint8_t data);//Fully filled

  esp_err_t oled_display_init();

  void IRAM_ATTR spi_event_callback(int event, void *arg);

  void display();
  void displaySSH1106();

  void setPixel(int16_t x, int16_t y) ;

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) ;

  void drawRect(int16_t x, int16_t y, int16_t width, int16_t height) ;
  void fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height) ;
  void drawCircle(int16_t x0, int16_t y0, int16_t radius);
  void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads);
  void fillCircle(int16_t x0, int16_t y0, int16_t radius) ;
  void drawHorizontalLine(int16_t x, int16_t y, int16_t length);
  void drawVerticalLine(int16_t x, int16_t y, int16_t length) ;
  void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) ;
  void drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t *data, uint16_t offset, uint16_t bytesInData);

  void drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth);

  void drawString(int16_t xMove, int16_t yMove, char* text) ;

  void oled_clear_dispaly_image();
  void oled_setContrast(uint8_t contrast, uint8_t precharge, uint8_t comdetect);
  void oled_setBrightness(uint8_t brightness);
  void oled_setFontSize(uint8_t font_Size/*Plain_10 , Plain_16 , Plain_24*/);
  void oled_setTextAlignment(uint8_t _textAlignment/*OLEDDISPLAY_TEXT_ALIGNMENT*/);


#endif

