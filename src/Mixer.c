#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <LUFA/Drivers/USB/USB.h>

#include "ADC.h"
#include "HID.h"

uint8_t potentiometerVal = 0;

int __attribute__((noreturn)) main(void) {
  wdt_disable();
  clock_prescale_set(clock_div_1);
  USB_Init();
  GlobalInterruptEnable();
  adc_init();

  for (;;) {
    potentiometerVal = adc_read(PF7);
    setPotentiometerValue(potentiometerVal);
    USB_USBTask();
    HID_Task();
  }
}
