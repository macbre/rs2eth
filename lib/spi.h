#ifndef __SPI_H
#define __SPI_H

#include "../rs_eth.h"

// inicjalizacja jako uk³adu master SPI
void spi_master_init(void);

// inicjalizacja jako uk³adu slave SPI
void spi_slave_init(void);

// odczyt/zapis SPI
unsigned char spi_read(void);
void spi_write(unsigned char data);

// wybór / deaktywacja wybranego uk³adu slave
#define spi_select(port, bit) 	    port &= ~_BV(bit); nop(); nop();	// !SS/CE = 0 (low)
#define spi_unselect(port, bit) 	port |= _BV(bit); nop(); nop(); 	// !SS/CE = 1 (hi)

// wybor czestotliwosci taktowania SPI (przy SPI2X)
#define spi_full_speed()            SPCR &= ~(0x03); SPCR |= (1<<SPI2X);                                // f_osc/2  -> 8MHz
#define spi_medium_speed()          SPCR &= ~(0x03); SPCR |= (1<<SPR0)|(1<<SPI2X);                      // f_osc/8  -> 2MHz
#define spi_low_speed()             SPCR &= ~(0x03); SPCR |= (1<<SPR1)|(1<<SPR0); SPSR &= ~(1<<SPI2X);  // f_osc/128 -> 125 kHz

#endif
