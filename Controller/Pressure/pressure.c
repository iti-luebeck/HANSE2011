#include "pressure.h"
#include "spi.h"

uint16_t C1, C2, C3, C4, C5, C6;
uint16_t D1, D2;

void calculateConfig( void) {
	C1 = ((W1 & 0xFFF8) >> 3);
	C2 = ((W1 & 0x0007) << 10) + ((W2 & 0xFFC0) >> 6);
	C3 = ((W3 & 0xFFC0) >> 6);
	C4 = ((W4 & 0xFF80) >> 7);
	C5 = ((W2 & 0x003F) << 6) + ((W3 & 0x003F));
	C6 = (W4 & 0x007F);
}

void calculatePressureTemp( void ) {


	uint16_t UT1, OFF, SENS, P, T, dT2;
	int16_t		dT;

	D1 = Pressure;
	D2 = Temperature;

	// Calculate calibration temperature
	UT1 = 8*C5+10000;

	// Calculate actual temperature
	dT = D2 - UT1;

	// Second-order temperature compensation
	if (dT < 0)
		dT2 = dT - (dT/128*dT/128)/2;
	else
		dT2 = dT - (dT/128*dT/128)/8;
	T = 200+dT2*(C6+100)/2048;

	// Calculate temperature compensated pressure
	OFF = C2+((C4-250)*dT)/4096+10000;

	SENS = C1/2 + ((C3+200)*dT)/8192 + 3000;

	// Temperature compensated pressure in mbar
	P = (SENS*(D1-OFF))/4096+1000;

	i2cdata[12] = MSB(P);
	i2cdata[13] = LSB(P);
	i2cdata[14] = MSB(T);
	i2cdata[15] = LSB(T);
	
}
