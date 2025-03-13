#ifndef ST7789_H
#define ST7789_H

#include <avr/io.h>

#define PIN(letter, number)                                                    \
  (__Pin_t) { .ddr = &DDR##letter, .port = &PORT##letter, .pin = number }

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320UL

#define HIGH(u16) ((uint8_t)(u16 >> 8))
#define LOW(u16) ((uint8_t)(u16))

// SPI commands
// https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/datasheet/unit/lcd/ST7789V2_SPEC_V1.0.pdf
#define MADCTL 0x36
#define SWRESET 0x10
#define SLPOUT 0x11
#define COLMOD 0x3A
#define INVON 0x21
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C

// RGB565 colors
#define WHITE 0xFFFF
#define RED 0xE062

typedef struct {
  volatile uint8_t *ddr;
  volatile uint8_t *port;
  uint8_t pin;
} __Pin_t;

typedef struct {
  __Pin_t CS;
  __Pin_t BLK;
  __Pin_t DC;
  __Pin_t RST;
} ST7789_t;

void ST7789_Init(ST7789_t *display);
void ST7789_ClearScreen(ST7789_t *display, uint16_t color);
void ST7789_FilledCircle(ST7789_t *display, uint16_t x, uint16_t y,
                         uint16_t radius, uint16_t color);

#endif
