#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "ADC.h"
#include "Descriptors.h"
#include "HID.h"
#include "ST7789.h"

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

void HID_CreateReport(uint8_t *buf, size_t size) {
  buf[0] = 0;                        // index
  buf[1] = 0;                        // report_type = UPDATE_VOLUME (0)
  buf[2] = (PIND & (1 << PD7)) == 0; // data = UP/DOWN
}

int __attribute__((noreturn)) main(void) {
  wdt_disable();
  clock_prescale_set(clock_div_1);
  USB_Init();
  GlobalInterruptEnable();
  adc_init();
  ST7789_Init(&lcd);
  ST7789_ClearScreen(&lcd, BLACK);

  for (;;) {
    // Endpoint_SelectEndpoint(HID_IN_EPADDR);
    // if (Endpoint_IsINReady()) {
    //   uint8_t in_buf[HID_EPSIZE] = {0};
    //   HID_CreateReport(in_buf, HID_EPSIZE);
    //   Endpoint_Write_Stream_LE(in_buf, HID_EPSIZE, NULL);
    //   Endpoint_ClearIN();
    // }
    USB_USBTask();
    // don't poll for hid reports if image is being drawn
    if (is_transmitting_image == false) {
      HID_Task();
    }
    Bulk_Task();
  }
}
