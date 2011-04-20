/*
 * main.c
 *
 *  Created on: Apr 5, 2011
 *      Author: robprak
 */
#include <avr/io.h>
#include <util/delay.h>
#include "usart_driver.h"

#define USART USARTF0

extern void get_adc_values();

extern volatile uint16_t adc_res_0, adc_res_1, adc_res_2, adc_res_3;
const uint16_t* adcs[] = {&adc_res_0, &adc_res_1, &adc_res_2, &adc_res_3};

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
	while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0) {}

	// PLL source XOSC, factor 2
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (2 << OSC_PLLFAC_gp);

	// Enable PLL
	OSC.CTRL |= OSC_PLLEN_bm;

	// wait for PLL to get ready
	while( (OSC.STATUS & OSC_PLLRDY_bm) == 0) {}

	// CLK.CTRL is protected, so we have to allow modification by the following command.
	CCP = CCP_IOREG_gc;
	// SCLKSEL is used to select the source for the System Clock.
	//CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;
}


/*
 * Send character via usart as soon as interface gets ready.
 */
void send_char(char c) {
	while ( !(USART.STATUS & USART_DREIF_bm) );
	USART.DATA = c;
}

void send_as_voltage(uint16_t* adcs[]) {
	for (int p=0; p<4; p++) {
		int16_t a;
		a = (*adcs[p] << 2);
		double x = a;
		x = x * 3.4 / 32768;
		int32_t y = x * 100000;
		char outchar[8];
		if (y >= 0) {
			outchar[0] = ' ';
		} else {
			y *= -1;
			outchar[0] = '-';
		}
		for (int i=7; i>1; i--) {
			int8_t r = y % 10;
			if (r<0) r += 10;
			outchar[i] = r + '0';
			y = y / 10;
		}
		outchar[1] = outchar[2];
		outchar[2] = '.';
		for (int i=0; i<8; i++) {
			send_char(outchar[i]);
		}

		if (p==3) {
			send_char('\r');
		} else {
			send_char(';');
		}
	}
}

/*
 * Initialize and enable USART.
 */
void init_usart() {
	// Pulling output of TXD0 high is very important. And how important it is.
	// Never remove this, or you will be unhappy!!! (Hint from XMega Datasheet.)
	PORTF.OUTSET   = PIN3_bm;

	// PC3 (TXD0) as output.
	PORTF.DIRSET   = PIN3_bm;

	// Set baudrate to 921600.
	// BSEL = 128, BSCALE = -7
	// BSEL = N[128*(2*14745600/(16*921600) - 1)]
	// error = N[2*14745600/(16*((2^(-7)*Floor[BSEL]) + 1))]/921600
	uint16_t bsel = 128;
    USART.BAUDCTRLA = (uint8_t)bsel;
    USART.BAUDCTRLB = (0x09 << USART_BSCALE0_bp) | (bsel >> 8);

	// Frame format: 8 data bits, no parity and one stop bit (8N1).
	USART.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;

    // enable Tx
	USART.CTRLB |= USART_TXEN_bm;
}

/*
 * Let led blink for testing.
 */
void led_blinking() {
	while (1) {
		_delay_ms(1000);
		PORTD.OUT |= PIN0_bm;
		_delay_ms(1000);
		PORTD.OUT &= ~PIN0_bm;
	}
}

/*
 * Send test pattern.
 */
void send_usart_test_sequence() {
//	uint16_t count = 0;
	char c = 'a';
	while (1) {
		send_char(c);
		c++;
		if (c>'z') {
			send_char('\r');
			send_char('\n');
			c='a';
		}
//		count++;
//		if (count > 65000) {
//			_delay_ms(1000);
//			count = 0;
//		}
	}
}

int main() {
	// green onboard led, low active
	PORTD.DIRSET = PIN0_bm;
	PORTD.OUTSET = PIN0_bm;
	// red error led, high active
	PORTD.DIRSET = PIN7_bm;

	// Enable external 16 MHz oscillator.
	set_external_oscillator();

	// For tests:
	// led_blinking();

	// Initialize USART.
	init_usart();

	// For tests:
	// send_usart_test_sequence();

	/* SPI */

	// define MOSI pin as output
	PORTC.DIRSET = PIN5_bm;
	// define SCK pin as output
	PORTC.DIRSET = PIN7_bm;
	// define CS pin as output
	PORTC.DIRSET = PIN3_bm;

	// define four MISO pins as input
	PORTC.DIRCLR = PIN6_bm;
	PORTC.DIRCLR = PIN0_bm;
	PORTC.DIRCLR = PIN1_bm;
	PORTC.DIRCLR = PIN2_bm;

	// Set CS high (idle).
	PORTC.OUTSET = PIN3_bm;
	// Be sure not to start the first adc conversion before idle state was processed.
	_delay_us(10);

	// Map ports for assembler.
	PORTCFG.VPCTRLA = PORTCFG_VP0MAP_PORTC_gc | PORTCFG_VP1MAP_PORTD_gc;

	PORTD.OUT &= ~PIN0_bm;
	while (1) {
		PORTD.OUTTGL = PIN0_bm;
		// receive data
		get_adc_values();
		send_as_voltage(adcs);

		_delay_ms(300);
	}

	return 0;
}
