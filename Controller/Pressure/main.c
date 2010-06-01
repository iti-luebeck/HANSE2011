#include "parameter.h"
#include "twislave.h"
#include "spi.h"
#include "pressure.h"

#include <util/delay.h>


void wait(uint8_t ms)  //loop: wartezeit in ms 
{
	uint8_t i;
	for(i=0;i<ms;i++) _delay_ms(1);
}



#define SLAVE_ADRESSE 0x50


int main (void) {
	init_twi_slave(SLAVE_ADRESSE);
	spi_init();
	calib = 1;

	while(1)  {
		if (calib) {
			spi_readCalibrationWords();
			calculateConfig();
			calib = 0;
		}

		//spi_reset();
		//wait(5);
		spi_startPressureMeasurement();
		wait(40);
		spi_stopPressureMeasurement();

		spi_startTemperatureMeasurement();
		wait(40);
		spi_stopTemperatureMeasurement();

		calculatePressureTemp();


		i2cdata[20]++;
	}
}
