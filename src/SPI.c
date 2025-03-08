#include "SPI.h"
#include <avr/io.h>

void SPI_Init(void) {
  // set MOSI, SCK and SS as outputs
  DDRB |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS);
  // MISO as input
  DDRB &= ~(1 << SPI_MISO);

  SPCR = (1 << SPE) | (1 << MSTR);
  SPCR |= (1 << SPI2X);
}

void SPI_Transfer(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)))
    ;
}
