#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "ADC.h"
#include "HID.h"
#include "ST7789.h"

uint16_t potentiometerVal = 0;

ST7789_t lcd = {
    .CS = PIN(B, PB6),
    .DC = PIN(B, PB4),
    .RST = PIN(B, PB5),
    .BLK = PIN(C, PC7),
};

int __attribute__((noreturn)) main(void) {
  wdt_disable();
  clock_prescale_set(clock_div_1);
  USB_Init();
  GlobalInterruptEnable();
  adc_init();
  ST7789_Init(&lcd);
  ST7789_ClearScreen(&lcd, WHITE);
  ST7789_FilledCircle(&lcd, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 60, RED);

  for (;;) {
    potentiometerVal = adc_read(PF7);
    setPotentiometerValue(potentiometerVal);
    USB_USBTask();
    HID_Task();
  }
}
