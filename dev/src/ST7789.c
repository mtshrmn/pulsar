#include "ST7789.h"
#include "SPI.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

#define __SETBIT(reg, bit) ((reg) |= (1 << (bit)))
#define __CLRBIT(reg, bit) ((reg) &= ~(1 << (bit)))

static inline void SETDDR(__Pin_t *p) { __SETBIT(*p->ddr, p->pin); }
static inline void SETPORT(__Pin_t *p) { __SETBIT(*p->port, p->pin); }
static inline void CLRPORT(__Pin_t *p) { __CLRBIT(*p->port, p->pin); }

static inline void SEND_CMD(ST7789_t *display, uint8_t cmd) {
  CLRPORT(&display->CS);
  CLRPORT(&DC);
  SPI_Transfer(cmd);
  SETPORT(&display->CS);
}

static inline void SEND_DATA(ST7789_t *display, uint8_t data) {
  CLRPORT(&display->CS);
  SETPORT(&DC);
  SPI_Transfer(data);
  SETPORT(&display->CS);
}

static inline void __configure_ddr(ST7789_t *display) {
  // set display pins as outputs
  SETDDR(&RST);
  SETDDR(&display->CS);
  // SETDDR(&display->BLK);
  SETDDR(&DC);
}

static inline void __configure_port(ST7789_t *display) {
  // set display pins as HIGH
  SETPORT(&RST);
  SETPORT(&display->CS);
  // SETPORT(&display->BLK);
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
  CLRPORT(&DC);
  SPI_Transfer(MADCTL);

  // set display mode to RGB565
  SETPORT(&DC);
  SPI_Transfer(0x00);

  SETPORT(&display->CS);
}

static inline void __reset_hw(ST7789_t *display) {
  // the reset sequence
  CLRPORT(&RST);
  _delay_us(15);
  SETPORT(&RST);
  _delay_ms(120);
}

static inline void __set_window(ST7789_t *display, uint16_t x0, uint16_t y0,
                                uint16_t x1, uint16_t y1) {
  CLRPORT(&DC);
  SPI_Transfer(CASET);

  SETPORT(&DC);
  SPI_Transfer(HIGH(x0));
  SPI_Transfer(LOW(x0));
  SPI_Transfer(HIGH(x1));
  SPI_Transfer(LOW(x1));

  CLRPORT(&DC);
  SPI_Transfer(RASET);

  SETPORT(&DC);
  SPI_Transfer(HIGH(y0));
  SPI_Transfer(LOW(y0));
  SPI_Transfer(HIGH(y1));
  SPI_Transfer(LOW(y1));
}

static inline void __set_color_small(ST7789_t *display, uint16_t color,
                                     uint8_t count) {

  CLRPORT(&DC);
  SPI_Transfer(RAMWR);
  SETPORT(&DC);
  while (count--) {
    SPI_Transfer(HIGH(color));
    SPI_Transfer(LOW(color));
  }
}

static inline void __set_color(ST7789_t *display, uint16_t color) {
  CLRPORT(&DC);
  SPI_Transfer(RAMWR);
  SETPORT(&DC);
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

void __draw_rect(ST7789_t *display, uint16_t color, uint16_t x0, uint16_t y0,
                 uint16_t x1, uint16_t y1) {
  CLRPORT(&display->CS);
  __set_window(display, x0, y0, x1, y1);

  uint16_t count = (x1 - x0 + 1) * (y1 - y0 + 1);
  if (count <= UINT8_MAX) {
    __set_color_small(display, color, count);
  } else {
    CLRPORT(&DC);
    SPI_Transfer(RAMWR);
    SETPORT(&DC);
    while (count--) {
      SPI_Transfer(HIGH(color));
      SPI_Transfer(LOW(color));
    }
  }

  SETPORT(&display->CS);
}

void ST7789_ClearScreen(ST7789_t *display, uint16_t color) {
  CLRPORT(&display->CS);
  __set_window(display, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  __set_color(display, color);
  SETPORT(&display->CS);
}

void ST7789_StartWriteRaw(ST7789_t *display, uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1) {
  CLRPORT(&display->CS);
  __set_window(display, x0, y0, x1, y1);
  // start of __set_color
  CLRPORT(&DC);
  SPI_Transfer(RAMWR);
  SETPORT(&DC);
}

void ST7789_WriteRaw(ST7789_t *display, uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    SPI_Transfer(data[i]);
  }
}

void ST7789_StopWriteRaw(ST7789_t *display) { SETPORT(&display->CS); }

void ST7789_DrawVolumeBar(ST7789_t *display) {
  __draw_rect(display, WHITE, 40, 240, 200, 242);
  __draw_rect(display, WHITE, 40, 258, 200, 260);
  __draw_rect(display, WHITE, 40, 240, 42, 260);
  __draw_rect(display, WHITE, 198, 240, 200, 260);
}

void ST7789_UpdateVolumeBar(ST7789_t *display, uint8_t volume,
                            uint8_t *prev_volume) {
  if (volume == *prev_volume) {
    return;
  }
  uint8_t volume_pos = (uint16_t)volume * 152 / 100 + 44;
  uint8_t prev_volume_pos = (uint16_t)*prev_volume * 152 / 100 + 44;
  if (volume > *prev_volume) {
    __draw_rect(display, WHITE, prev_volume_pos, 244, volume_pos, 256);
  } else {
    __draw_rect(display, BLACK, volume_pos, 244, prev_volume_pos, 256);
  }

  *prev_volume = volume;
}
