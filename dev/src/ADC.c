#include "ADC.h"

uint16_t adc_read(uint8_t channel) {
  ADMUX = (1 << REFS0) | (channel & 0x07);
  ADCSRA |= (1 << ADSC);
  while ((ADCSRA & (1 << ADIF)) == 0)
    ;
  ADCSRA |= (1 << ADIF);
  return (ADCL | ADCH << 8);
}

void adc_init(void) {
  ADMUX = (1 << REFS0); // For Aref=AVcc;
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
}
