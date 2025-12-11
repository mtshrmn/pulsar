#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"
#include "HID.h"
#include "ST7789.h"

#define SET_ISR(id)                                                            \
  ISR(INT##id##_vect) { handle_interrupt(id); }

ST7789_t displays[NUM_DISPLAYS] = {
    PIN(F, PF0),
    PIN(F, PF1),
    PIN(F, PF4),
    PIN(F, PF5),
};

bool is_transmitting_image = false;
uint32_t len = 0;
ImageData image_data;
HIDReport hid_report;
uint8_t prev_volumes[NUM_DISPLAYS] = {0};

static inline void encoder_init(void) {
  // set PD0, PD1, PD2, PD3, PD7 to input.
  DDRD &= ~_BV(PD0) & ~_BV(PD1) & ~_BV(PD2) & ~_BV(PD3) & ~_BV(PD7);
  // do not use internal pullup resistor on those ports
  PORTD &= ~_BV(PD0) & ~_BV(PD1) & ~_BV(PD2) & ~_BV(PD3) & ~_BV(PD7);

  // EICRA has 4 set of INT pins.
  // each set should be set to 0b10 for falling edge detection.
  EICRA = 0b10101010;
  // set EIMSK lower half to high to enable corresponding ISRs (INT0-INT4).
  EIMSK |= 0b1111;

  sei();
}

void handle_interrupt(uint8_t index) {
  uint8_t dt = (PIND & (1 << PD7));
  cli();

  Endpoint_SelectEndpoint(HID_IN_EPADDR);
  if (Endpoint_IsINReady()) {
    HIDReport report = {
        .index = index,
        .report_type = dt ? REPORT_TYPE_VOLUME_INC : REPORT_TYPE_VOLUME_DEC,
    };
    Endpoint_Write_Stream_LE((uint8_t *)&report, sizeof(report), NULL);
    Endpoint_ClearIN();
  }

  sei();
}

SET_ISR(0)
SET_ISR(1)
SET_ISR(2)
SET_ISR(3)

void Bulk_ProcessData(uint8_t *buf, size_t size) {
  if (is_transmitting_image == false) {
    is_transmitting_image = true;
    len = 0;
    image_data = *(ImageData *)buf;
    // for now, assert image_data.index == 0
    // clang-format off
    ST7789_StartWriteRaw(&displays[image_data.index], image_data.x0, image_data.y0,
                         image_data.x1, image_data.y1);
    // clang-format on

    return;
  }

  len += size;
  ST7789_WriteRaw(&displays[image_data.index], buf, size);

  if (len >= image_data.data_len) {
    is_transmitting_image = false;
    ST7789_StopWriteRaw(&displays[image_data.index]);
    return;
  }
}

void HID_ProcessReport(uint8_t *report, size_t size) {
  hid_report = *(HIDReport *)report;
  uint8_t expected_parity =
      hid_report.index ^ hid_report.report_type ^ hid_report.volume;

  if (hid_report.parity != expected_parity) {
    HID_ReportACK();
    return;
  }

  ST7789_t *display = &displays[hid_report.index];
  uint8_t *prev_volume = &prev_volumes[hid_report.index];
  // assert hid_report.index == 0
  switch (hid_report.report_type) {
  case REPORT_TYPE_SET_VOLUME:
    ST7789_UpdateVolumeBar(display, hid_report.volume, prev_volume);
    break;
  case REPORT_TYPE_INIT:
    ST7789_DrawVolumeBar(display);
    ST7789_UpdateVolumeBar(display, 0, prev_volume);
    break;
  case REPORT_TYPE_CLEAR:
    ST7789_ClearScreen(display, BLACK);
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
  ST7789_Init(displays);
  ST7789_ClearScreens(displays, BLACK);
  // for (size_t i = 0; i < NUM_DISPLAYS; ++i) {
  //   ST7789_ClearScreen(&displays[i], WHITE);
  // }

  for (;;) {
    USB_USBTask();
    // don't poll for hid reports if image is being drawn
    if (is_transmitting_image == false) {
      HID_Task();
    }
    Bulk_Task();
  }
}
