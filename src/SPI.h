#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

#define SPI_SCK PB1
#define SPI_MISO PB3
#define SPI_MOSI PB2
#define SPI_SS PB0

void SPI_Init(void);
void SPI_Transfer(uint8_t cmd);

#endif
