#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "Descriptors.h"
#include "HID.h"
#include "ST7789.h"

ST7789_t displays[NUM_DISPLAYS] = {
    {.CS = PIN(F, PF0)},
    {.CS = PIN(F, PF1)},
    {.CS = PIN(F, PF4)},
    {.CS = PIN(F, PF5)},
};

bool is_transmitting_image = false;
uint32_t len = 0;
ImageData image_data;
HIDReport hid_report;
uint8_t prev_volumes[NUM_DISPLAYS] = {0};

static inline void encoder_init(void) {
  DDRD &= ~(1 << PD0);
  DDRD &= ~(1 << PD1);
  DDRD &= ~(1 << PD2);
  DDRD &= ~(2 << PD3);
  DDRD &= ~(1 << PD6);

  PORTD &= ~(1 << PD0);
  PORTD &= ~(1 << PD1);
  PORTD &= ~(1 << PD2);
  PORTD &= ~(1 << PD3);
  PORTD &= ~(1 << PD7);

  EICRA |= ~(1 << ISC01);
  EICRA |= ~(1 << ISC00);
  EICRA |= ~(1 << ISC11);
  EICRA |= ~(1 << ISC10);
  EICRA |= ~(1 << ISC21);
  EICRA |= ~(1 << ISC20);
  EICRA |= ~(1 << ISC31);
  EICRA |= ~(1 << ISC30);
  EIMSK |= (1 << INT0);
  EIMSK |= (1 << INT1);
  EIMSK |= (1 << INT2);
  EIMSK |= (1 << INT3);

  sei();
}

void handle_interrupt(uint8_t index) {
  uint8_t dt = (PIND & (1 << PD6));
  cli();

  Endpoint_SelectEndpoint(HID_IN_EPADDR);
  if (Endpoint_IsINReady()) {
    HIDReport report = {
        .index = index,
        .report_type = dt ? REPORT_TYPE_VOLUME_DEC : REPORT_TYPE_VOLUME_INC,
    };
    Endpoint_Write_Stream_LE((uint8_t *)&report, sizeof(report), NULL);
    Endpoint_ClearIN();
  }

  sei();
}

ISR(INT0_vect) { handle_interrupt(0); }
ISR(INT1_vect) { handle_interrupt(1); }
ISR(INT2_vect) { handle_interrupt(2); }
ISR(INT3_vect) { handle_interrupt(3); }

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
  ST7789_t *display = &displays[hid_report.index];
  uint8_t *prev_volume = prev_volumes + hid_report.index;
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
