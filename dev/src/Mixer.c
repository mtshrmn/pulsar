#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "ADC.h"
#include "Descriptors.h"
#include "HID.h"
#include "ST7789.h"

uint16_t potentiometerVal = 0;

ST7789_t lcd = {
    .CS = PIN(B, PB6),
    .DC = PIN(B, PB4),
    .RST = PIN(B, PB5),
    .BLK = PIN(C, PC7),
};

bool is_transmitting_image = false;
uint32_t len = 0;
ImageData image_data;
uint16_t color = WHITE;
uint8_t i = 0;
uint8_t volume = 0;
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
  if (report[0] != 0) {
    return;
  }

  if (report[1] == 1) {
    ST7789_ClearScreen(&lcd, WHITE);
    return;
  }

  if (report[2]) {
    if (volume > 0) {
      volume--;
    }
  } else {
    if (volume < 100) {
      volume++;
    }
  }

  ST7789_UpdateVolumeBar(&lcd, volume, &prev_volume);
}

void HID_CreateReport(uint8_t *buf, size_t size) {
  buf[0] = 0;                        // index
  buf[1] = 0;                        // report_type = UPDATE_VOLUME (0)
  buf[2] = (PIND & (1 << PD7)) == 0; // data = UP/DOWN
}

void setup_external_interrupt(void) {
  // set PD1 to input pullup.
  DDRD &= ~(1 << PD1);
  PORTD |= (1 << PD1);

  DDRD &= ~(1 << PD7);
  PORTD |= (1 << PD7);

  // interrupt on falling edge
  EICRA |= ~(1 << ISC11);
  EICRA |= ~(1 << ISC10);
  // enable interrupt in mask
  EIMSK |= (1 << INT1);
}

ISR(INT1_vect) {
  // HID IN
  Endpoint_SelectEndpoint(HID_IN_EPADDR);
  if (Endpoint_IsINReady()) {
    uint8_t in_buf[HID_EPSIZE] = {0};
    HID_CreateReport(in_buf, HID_EPSIZE);
    Endpoint_Write_Stream_LE(in_buf, HID_EPSIZE, NULL);
    Endpoint_ClearIN();
  }
}

int __attribute__((noreturn)) main(void) {
  wdt_disable();
  clock_prescale_set(clock_div_1);
  USB_Init();
  GlobalInterruptEnable();
  setup_external_interrupt();
  adc_init();
  ST7789_Init(&lcd);
  ST7789_ClearScreen(&lcd, WHITE);

  for (;;) {
    // potentiometerVal = adc_read(PF7);
    // ST7789_FilledCircle(&lcd, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 60,
    //                     potentiometerVal);
    USB_USBTask();
    HID_Task();
    Bulk_Task();
  }
}

// WARNING: =================================================================

// ISR(INT1_vect) {
//   // ST7789_FilledCircle(&lcd, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 60,
//   RED); if (PIND & (1 << PD7)) {
//     // ccw
//     if (volume > 0) {
//       volume--;
//     }
//   } else {
//     // cw
//     if (volume < 100) {
//       volume++;
//     }
//   }
//   ST7789_UpdateVolumeBar(&lcd, volume, &prev_volume);
// }

int __attribute__((noreturn)) notmain(void) {
  clock_prescale_set(clock_div_1);
  GlobalInterruptEnable();
  setup_external_interrupt();
  ST7789_Init(&lcd);
  ST7789_ClearScreen(&lcd, WHITE);
  ST7789_DrawVolumeBar(&lcd);

  for (;;) {
    // do nothing.
  }
}
