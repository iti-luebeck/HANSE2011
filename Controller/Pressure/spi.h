/*#######################################################################################
Connect ARM to MMC/SD 

Copyright (C) 2004 Ulrich Radig
#######################################################################################*/

#ifndef _SPI_H_
 #define _SPI_H_

#include <avr/io.h>	
#include "makros.h"
#include "parameter.h"


#define MMC_Write			PORTB
#define MMC_Read			PINB
#define MMC_Direction_REG	DDRB



#define SPI_DI				4
#define SPI_DO				3
#define SPI_Clock			5
#define SPI_SlaveSelect		2


extern volatile uint16_t W1, W2, W3, W4;
extern volatile uint16_t Pressure;
extern volatile uint16_t Temperature;



void spi_init(void);


void spi_readCalibrationWords(void);

void spi_reset();




void spi_startPressureMeasurement();

void spi_stopPressureMeasurement();

void spi_startTemperatureMeasurement();

void spi_stopTemperatureMeasurement();

unsigned char spi_read_byte(void);

void spi_write_byte(unsigned char);


#define nop()  __asm__ __volatile__ ("nop" ::)

#endif


