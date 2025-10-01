#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"
#include "HID.h"
#include "ST7789.h"

// given a prescaler of 64, 1 tick = 4us.
// 2500 ticks = 10ms
#define DEBOUNCE_TICKS 2500

ST7789_t lcd = {
    .CS = PIN(B, PB6),
    .DC = PIN(B, PB4),
    .RST = PIN(B, PB5),
    .BLK = PIN(C, PC7),
};

bool is_transmitting_image = false;
uint32_t len = 0;
ImageData image_data;
HIDReport hid_report;
uint8_t prev_volume = 0;
static volatile uint16_t last_interrupt = 0;

static inline void encoder_init(void) {
  DDRD &= ~(1 << PD1);
  DDRD &= ~(1 << PD7);

  PORTD &= ~(1 << PD1);
  PORTD &= ~(1 << PD7);

  EICRA |= ~(1 << ISC11);
  EICRA |= ~(1 << ISC10);
  EIMSK |= (1 << INT1);

  // set timer1 prescaler to 64
  TCCR1B = (1 << CS11) | (1 << CS10);
  sei();
}

ISR(INT1_vect) {
  cli();
  uint8_t clk = (PIND & (1 << PD1)) ? 1 : 0;
  uint8_t dt = (PIND & (1 << PD7)) ? 1 : 0;

  uint16_t now = TCNT1;
  uint16_t elapsed = now - last_interrupt;

  if (elapsed < DEBOUNCE_TICKS) {
    return;
  }
  last_interrupt = now;

  Endpoint_SelectEndpoint(HID_IN_EPADDR);
  if (Endpoint_IsINReady()) {
    HIDReport report = {
        .report_type =
            clk != dt ? REPORT_TYPE_VOLUME_INC : REPORT_TYPE_VOLUME_DEC,
    };
    Endpoint_Write_Stream_LE((uint8_t *)&report, sizeof(report), NULL);
    Endpoint_ClearIN();
  }
  sei();
}

void Bulk_ProcessData(uint8_t *buf, size_t size) {
  if (is_transmitting_image == false) {
    is_transmitting_image = true;
    len = 0;
    image_data = *(ImageData *)buf;
    // for now, assert image_data.index == 0
    // clang-format off
    ST7789_StartWriteRaw(&lcd, image_data.x0, image_data.y0,
                         image_data.x1, image_data.y1);
    // clang-format on

    return;
  }

  len += size;
  ST7789_WriteRaw(&lcd, buf, size);

  if (len >= image_data.data_len) {
    is_transmitting_image = false;
    ST7789_StopWriteRaw(&lcd);
    return;
  }
}

void HID_ProcessReport(uint8_t *report, size_t size) {
  hid_report = *(HIDReport *)report;
  // assert hid_report.index == 0
  switch (hid_report.report_type) {
  case REPORT_TYPE_SET_VOLUME:
    ST7789_UpdateVolumeBar(&lcd, hid_report.volume, &prev_volume);
    break;
  case REPORT_TYPE_INIT:
    ST7789_DrawVolumeBar(&lcd);
    ST7789_UpdateVolumeBar(&lcd, 0, &prev_volume);
    break;
  case REPORT_TYPE_CLEAR:
    ST7789_ClearScreen(&lcd, BLACK);
    break;
  default:
    break;
  }
  HID_ReportACK();
}

int __attribute__((noreturn)) main(void) {
  wdt_disable();
  clock_prescale_set(clock_div_1);
  USB_Init();
  GlobalInterruptEnable();
  encoder_init();
  ST7789_Init(&lcd);
  ST7789_ClearScreen(&lcd, BLACK);

  for (;;) {
    USB_USBTask();
    // don't poll for hid reports if image is being drawn
    if (is_transmitting_image == false) {
      HID_Task();
    }
    Bulk_Task();
  }
}
