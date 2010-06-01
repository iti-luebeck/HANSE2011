
#include "spi.h"

volatile uint16_t W1, W2, W3, W4;
volatile uint16_t Pressure = 0;
volatile uint16_t Temperature = 0;



void spi_init () {	
	MMC_Direction_REG &=~(1<<SPI_DI);
	MMC_Direction_REG |= (1<<SPI_Clock);
	MMC_Direction_REG |= (1<<SPI_DO);
	MMC_Direction_REG |= (1<<SPI_SlaveSelect);
	MMC_Write &=~(1<<SPI_Clock); 

	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1); //Enable SPI, SPI in Master Mode	
	SPSR = (1<<SPI2X);
}



void spi_readCalibrationWords() {

	spi_write_byte( 0x1D );
	spi_write_byte( 0x50 );
	MSB(W1) = spi_read_byte();
	LSB(W1) = spi_read_byte();

	spi_write_byte( 0x1D );
	spi_write_byte( 0x60 );
	MSB(W2) = spi_read_byte();
	LSB(W2) = spi_read_byte();

	spi_write_byte( 0x1D );
	spi_write_byte( 0x90 );
	MSB(W3) = spi_read_byte();
	LSB(W3) = spi_read_byte();

	spi_write_byte( 0x1D );
	spi_write_byte( 0xA0 );
	MSB(W4) = spi_read_byte();
	LSB(W4) = spi_read_byte();

	i2cdata[0] = MSB(W1);
	i2cdata[1] = LSB(W1);
	i2cdata[2] = MSB(W2);
	i2cdata[3] = LSB(W2);
	i2cdata[4] = MSB(W3);
	i2cdata[5] = LSB(W3);
	i2cdata[6] = MSB(W4);
	i2cdata[7] = LSB(W4);
}

void spi_reset() {
	spi_write_byte( 0x15 );
	spi_write_byte( 0x55 );
	spi_write_byte( 0x40 );
}



void spi_startPressureMeasurement() {
	spi_write_byte( 0x0F );
	spi_write_byte( 0x40 );
}

void spi_stopPressureMeasurement() {
	MSB( Pressure ) = spi_read_byte();
	LSB( Pressure ) = spi_read_byte();
	i2cdata[8] = MSB( Pressure );
	i2cdata[9] = LSB( Pressure );
}



void spi_startTemperatureMeasurement() {
	spi_write_byte( 0x0F );
	spi_write_byte( 0x20 );
}

void spi_stopTemperatureMeasurement() {
	MSB( Temperature ) = spi_read_byte();
	LSB( Temperature ) = spi_read_byte();
	i2cdata[10] = MSB( Temperature );
	i2cdata[11] = LSB( Temperature );
}



unsigned char spi_read_byte (void) 
{
	SPCR |= (1<<CPHA);
	uint8_t Byte = 0;
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF))) {
		;
	}
	Byte = SPDR;
	SPCR &=~(1<<CPHA);
	return (Byte);
}

void spi_write_byte (unsigned char Byte) 
{
	SPDR = Byte; 					//Sendet ein Byte
	while(!(SPSR & (1<<SPIF))) { 	//Wartet bis Byte gesendet wurde
		;
	}

}

