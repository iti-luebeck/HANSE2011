/*
 * main.c
 *
 *  Created on: Apr 5, 2011
 *      Author: robprak
 */
// #include <inttypes.h>
#include <avr/io.h>
// #include <avr/sleep.h>
#include <util/delay.h>

/*
 * Enable external 16 MHz oscillator.
 */
void set_external_oscillator() {
	// These bits select the type and start-up time for the crystal or resonator that is
	// connected to the	XTAL or TOSC pins.
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;

	// Enable external clock and disable everything else.
    OSC.CTRL = OSC_XOSCEN_bm;

	// Page 87: The	external clock source should be allowed time to become stable before
	// it is selected as source for	the System Clock.
	while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0) {
//	  PORTD.OUTTGL = PIN0_bm;
	}

	// CLK.CTRL is protected, so we have to allow modification by the following command.
	CCP = CCP_IOREG_gc;
	// SCLKSEL is used to select the source for the System Clock.
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
}

int main() {
	PORTD.DIRSET |= PIN0_bm;
	PORTD.OUT |= PIN0_bm;

	set_external_oscillator();

	while (1) {
		_delay_ms(1000);
		PORTD.OUT |= PIN0_bm;
		_delay_ms(1000);
		PORTD.OUT &= ~PIN0_bm;
	}
	return 0;
}
