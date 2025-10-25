#include "ST7789.h"
#include "SPI.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

#define SETBIT(reg, bit) ((reg) |= _BV(bit))
#define CLRBIT(reg, bit) ((reg) &= ~_BV(bit))

#define FOR_DISPLAY(iter) for (size_t iter = 0; iter < NUM_DISPLAYS; ++iter)

static inline void SETDDR(const __Pin_t *const p) { SETBIT(*p->ddr, p->pin); }
static inline void SETPORT(const __Pin_t *const p) { SETBIT(*p->port, p->pin); }
static inline void CLRPORT(const __Pin_t *const p) { CLRBIT(*p->port, p->pin); }

static const __Pin_t DC = PIN(F, PF6);
static const __Pin_t RST = PIN(F, PF7);

static inline void configure_ddrs(ST7789_t displays[NUM_DISPLAYS]) {
  SETDDR(&RST);
  FOR_DISPLAY(i) { SETDDR(&displays[i].CS); }
  SETDDR(&DC);
}

static inline void configure_ports(ST7789_t displays[NUM_DISPLAYS]) {
  SETPORT(&RST);
  FOR_DISPLAY(i) { SETPORT(&displays[i].CS); }
}

static inline void init_sequence(ST7789_t displays[NUM_DISPLAYS]) {
  FOR_DISPLAY(i) { CLRPORT(&displays[i].CS); }

  CLRPORT(&DC);
  SPI_Transfer(SWRESET);
  _delay_ms(150);

  SPI_Transfer(SLPOUT);
  _delay_ms(150);

  SPI_Transfer(COLMOD);
  SETPORT(&DC);
  SPI_Transfer(WRCACE);
  _delay_ms(10);

  CLRPORT(&DC);
  SPI_Transfer(INVON);
  _delay_ms(150);
  SPI_Transfer(DISPON);
  _delay_ms(200);

  FOR_DISPLAY(i) { SETPORT(&displays[i].CS); }
}

static inline void madctl(ST7789_t displays[NUM_DISPLAYS]) {
  FOR_DISPLAY(i) { CLRPORT(&displays[i].CS); }
  CLRPORT(&DC);
  SPI_Transfer(MADCTL);
  // set display mode to RGB565
  SETPORT(&DC);
  SPI_Transfer(0x00);
  FOR_DISPLAY(i) { SETPORT(&displays[i].CS); }
}

static inline void reset_hw(void) {
  CLRPORT(&RST);
  _delay_us(15);
  SETPORT(&RST);
  _delay_ms(120);
}

void ST7789_Init(ST7789_t displays[NUM_DISPLAYS]) {
  SPI_Init();
  configure_ddrs(displays);
  configure_ports(displays);
  _delay_ms(10);

  reset_hw();
  init_sequence(displays);
  madctl(displays);
}

static inline void set_window(uint16_t x0, uint16_t y0, uint16_t x1,
                              uint16_t y1) {
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

static inline void set_fullscreen_color(uint16_t color) {
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

void ST7789_ClearScreen(ST7789_t *display, uint16_t color) {
  CLRPORT(&display->CS);
  set_window(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  set_fullscreen_color(color);
  SETPORT(&display->CS);
}

void ST7789_ClearScreens(ST7789_t displays[NUM_DISPLAYS], uint16_t color) {
  FOR_DISPLAY(i) { CLRPORT(&displays[i].CS); }
  set_window(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  set_fullscreen_color(color);
  FOR_DISPLAY(i) { SETPORT(&displays[i].CS); }
}

void ST7789_StartWriteRaw(ST7789_t *display, uint16_t x0, uint16_t y0,
                          uint16_t x1, uint16_t y1) {
  CLRPORT(&display->CS);
  set_window(x0, y0, x1, y1);
  // start of __set_color
  CLRPORT(&DC);
  SPI_Transfer(RAMWR);
  SETPORT(&DC);
}

// TODO: optimise this with inline assembly
void ST7789_WriteRaw(ST7789_t *display, uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    SPI_Transfer(data[i]);
  }
}

void ST7789_StopWriteRaw(ST7789_t *display) { SETPORT(&display->CS); }

static inline void set_color(uint16_t color, uint16_t count) {
  CLRPORT(&DC);
  SPI_Transfer(RAMWR);
  SETPORT(&DC);
  while (count--) {
    SPI_Transfer(HIGH(color));
    SPI_Transfer(LOW(color));
  }
}

static inline void draw_rect(uint16_t color, uint16_t x0, uint16_t y0,
                             uint16_t x1, uint16_t y1) {
  uint16_t count = (x1 - x0 + 1) * (y1 - y0 + 1);
  set_window(x0, y0, x1, y1);
  set_color(color, count);
}

void ST7789_DrawVolumeBar(ST7789_t *display) {
  CLRPORT(&display->CS);
  draw_rect(WHITE, 40, 240, 200, 242);
  draw_rect(WHITE, 40, 258, 200, 260);
  draw_rect(WHITE, 40, 240, 42, 260);
  draw_rect(WHITE, 198, 240, 200, 260);
  SETPORT(&display->CS);
}

static void update_volume_regular(ST7789_t *display, uint8_t volume,
                                  uint8_t *prev_volume) {
  if (volume == *prev_volume) {
    return;
  }
  uint8_t volume_pos = (uint16_t)volume * 152 / 100 + 44;
  uint8_t prev_volume_pos = (uint16_t)*prev_volume * 152 / 100 + 44;
  CLRPORT(&display->CS);
  if (volume > *prev_volume) {
    draw_rect(WHITE, prev_volume_pos, 244, volume_pos, 256);
  } else {
    draw_rect(BLACK, volume_pos, 244, prev_volume_pos, 256);
  }
  SETPORT(&display->CS);

  *prev_volume = volume;
}

static void update_volume_loud(ST7789_t *display, uint8_t volume,
                               uint8_t *prev_volume) {
  if (volume == *prev_volume) {
    return;
  }

  // nasty linear equation
  uint8_t volume_pos = ((uint16_t)volume * 152 - 13000) / 50;
  uint8_t prev_volume_pos = ((uint16_t)*prev_volume * 152 - 13000) / 50;
  CLRPORT(&display->CS);
  if (volume > *prev_volume) {
    draw_rect(RED, prev_volume_pos, 244, volume_pos, 256);
  } else {
    draw_rect(WHITE, volume_pos, 244, prev_volume_pos, 256);
  }
  SETPORT(&display->CS);

  *prev_volume = volume;
}

void ST7789_UpdateVolumeBar(ST7789_t *display, uint8_t volume,
                            uint8_t *prev_volume) {
  if (volume > 100 && *prev_volume > 100) {
    update_volume_loud(display, volume, prev_volume);
    return;
  }

  if (volume <= 100 && *prev_volume <= 100) {
    update_volume_regular(display, volume, prev_volume);
    return;
  }

  if (volume > *prev_volume) {
    // volume is > 100 while prev_volume <= 100
    update_volume_regular(display, 100, prev_volume);
    update_volume_loud(display, volume, prev_volume);
  } else {
    update_volume_loud(display, 100, prev_volume);
    update_volume_regular(display, volume, prev_volume);
  }
}
