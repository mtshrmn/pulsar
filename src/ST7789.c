#include "ST7789.h"
#include "SPI.h"
#include <util/delay.h>

#define __SETBIT(reg, bit) ((reg) |= (1 << (bit)))
#define __CLRBIT(reg, bit) ((reg) &= ~(1 << (bit)))

static inline void SETDDR(__Pin_t *p) { __SETBIT(*p->ddr, p->pin); }
static inline void SETPORT(__Pin_t *p) { __SETBIT(*p->port, p->pin); }
static inline void CLRPORT(__Pin_t *p) { __CLRBIT(*p->port, p->pin); }

static inline void SEND_CMD(ST7789_t *display, uint8_t cmd) {
  CLRPORT(&display->CS);
  CLRPORT(&display->DC);
  SPI_Transfer(cmd);
  SETPORT(&display->CS);
}

static inline void SEND_DATA(ST7789_t *display, uint8_t data) {
  CLRPORT(&display->CS);
  SETPORT(&display->DC);
  SPI_Transfer(data);
  SETPORT(&display->CS);
}

static inline void __configure_ddr(ST7789_t *display) {
  // set display pins as outputs
  SETDDR(&display->RST);
  SETDDR(&display->CS);
  SETDDR(&display->BLK);
  SETDDR(&display->DC);
}

static inline void __configure_port(ST7789_t *display) {
  // set display pins as HIGH
  SETPORT(&display->RST);
  SETPORT(&display->CS);
  SETPORT(&display->BLK);
}

static inline void __init_sequence(ST7789_t *display) {
  SEND_CMD(display, SWRESET);
  _delay_ms(150);

  SEND_CMD(display, SLPOUT);
  _delay_ms(150);

  SEND_CMD(display, COLMOD);
  SEND_DATA(display, 0x55);
  _delay_ms(10);

  SEND_CMD(display, INVON);
  _delay_ms(150);

  SEND_CMD(display, DISPON);
  _delay_ms(200);
}

static inline void __madctl(ST7789_t *display) {
  CLRPORT(&display->CS);

  // set memory address access control
  CLRPORT(&display->DC);
  SPI_Transfer(MADCTL);

  // set display mode to RGB565
  SETPORT(&display->DC);
  SPI_Transfer(0x00);

  SETPORT(&display->CS);
}

static inline void __reset_hw(ST7789_t *display) {
  // the reset sequence
  CLRPORT(&display->RST);
  _delay_us(15);
  SETPORT(&display->RST);
  _delay_ms(120);
}

static inline void __set_window(ST7789_t *display) {
  CLRPORT(&display->DC);
  SPI_Transfer(CASET);

  SETPORT(&display->DC);
  SPI_Transfer(HIGH(0));
  SPI_Transfer(LOW(0));
  SPI_Transfer(HIGH(SCREEN_WIDTH));
  SPI_Transfer(LOW(SCREEN_WIDTH));

  CLRPORT(&display->DC);
  SPI_Transfer(RASET);

  SETPORT(&display->DC);
  SPI_Transfer(HIGH(0));
  SPI_Transfer(LOW(0));
  SPI_Transfer(HIGH(SCREEN_HEIGHT));
  SPI_Transfer(LOW(SCREEN_HEIGHT));
}

static inline void __set_color(ST7789_t *display, uint16_t color) {
  CLRPORT(&display->DC);
  SPI_Transfer(RAMWR);
  SETPORT(&display->DC);
  // since uint8_t cant hold (SCREEN_WIDTH * SCREEN_HEIGHT)
  // using a simple for-loop will be a slowdown
  // instead, an ugly hack is being used

  // first do 256*256 iterations
  for (uint8_t i = 0; i < 0xFF; ++i) {
    for (uint8_t j = 0; j < 0xFF; ++j) {
      SPI_Transfer(HIGH(color));
      SPI_Transfer(LOW(color));
    }
  }

  // assuming remainder can hold that number
  uint8_t remainder = (SCREEN_HEIGHT * SCREEN_WIDTH / 256) - 256;
  for (uint8_t i = 0; i < remainder; ++i) {
    for (uint8_t j = 0; j < 0xFF; ++j) {
      SPI_Transfer(HIGH(color));
      SPI_Transfer(LOW(color));
    }
  }
}

void ST7789_Init(ST7789_t *display) {
  SPI_Init();
  __configure_ddr(display);
  __configure_port(display);
  _delay_ms(10);

  __reset_hw(display);
  __init_sequence(display);
  __madctl(display);
}

void ST7789_ClearScreen(ST7789_t *display, uint16_t color) {
  CLRPORT(&display->CS);
  __set_window(display);
  __set_color(display, color);
  SETPORT(&display->CS);
}
