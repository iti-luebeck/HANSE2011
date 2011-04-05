/*
 * main.c
 *
 *  Created on: Apr 5, 2011
 *      Author: robprak
 */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>


int main() {
	  	  CCP = 0xD8;
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_256CLK_gc;
    OSC.CTRL |= OSC_XOSCEN_bm;
    OSC.CTRL &= ~OSC_RC2MEN_bm;
    OSC.CTRL &= ~OSC_RC32MEN_bm;

	  //wait for osc to become ready
//	  while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0){
//	    //PORTF.OUTTGL = (1<<6);
//	    PORTK.OUTTGL = (1<<0); //toggle led   //toggle led
//	  }

	PORTD.DIRSET |= PIN0_bm;
	PORTD.OUT &= ~PIN0_bm;
	while (1) {
		_delay_ms(1000);
		PORTD.OUT |= PIN0_bm;
		_delay_ms(1000);
		PORTD.OUT &= ~PIN0_bm;
	}
	return 0;
}
