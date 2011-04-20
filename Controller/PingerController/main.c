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
uint8_t sync_char_cnt;
uint8_t usart_buffer_bytes;
uint8_t usart_buffer[4];

char volatile read_request;

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
	// without interrupts
	while ( !(USART.STATUS & USART_DREIF_bm) );
	USART.DATA = c;
	// with interrupts
//	while ( usart_buffer_bytes == 3 ) {};
//	usart_buffer[usart_buffer_bytes++] = c;
//	USART.CTRLA |= (USART.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_LO_gc;
}

/*
 * Send data as raw bytes. High byte will not be transmitted, but taken care of for senseful lowbyte-only information.
 */
void send_as_single_byte_sequence() {
	sync_char_cnt++;
	// sync char every x*4 bytes
	if (sync_char_cnt == 4) {
		send_char(-128);
		sync_char_cnt = 0;

	}
	for (int p=0; p<4; p++) {
		int8_t a = (int8_t)(*adcs[p] >> 8);
		int8_t b = (int8_t)(*adcs[p]) & 0b01111111;
		if (a<0) {
			b |= 0x80;
		}
		if (a != 0) {
			if (a > 0) {
				b = 127;
			} else {
				b = -127;
			}
		}
		if (b == -128) {
			b = -127;
		}
		send_char(b);
	}
}

/*
 * Send data as nice ASCII string containing voltage information.
 */
void send_as_voltage() {
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
 * Initialize SPI for ADC chips.
 */
void init_spi() {
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

	// Set CS high (idle state).
	PORTC.OUTSET = PIN3_bm;
	// Be sure not to start the first ADC conversion before idle state was processed.
	_delay_us(10);
}

/*
 * Initialize and start timer.
 */
void init_and_start_timer(TC_CLKSEL_t clockSelection, uint16_t period) {
	// period
    TCC0.PER =  period;
//    TCC1.PER =  65535;
    // prescaler
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | clockSelection;
//	TCC1.CTRLA = ( TCC1.CTRLA & ~TC1_CLKSEL_gm ) | clockSelection;
    // Timer Mode: Normal
    TCC0.CTRLB = 0x00;
//    TCC1.CTRLB = 0x00;
	// Configure as high level interrupt.
	TCC0.INTCTRLA = TC0_OVFINTLVL_gm;
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
	sync_char_cnt = 0;
	usart_buffer_bytes = 0;

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

	// Initialize SPI for ADC chips.
	init_spi();

	// Map ports for assembler.
	PORTCFG.VPCTRLA = PORTCFG_VP0MAP_PORTC_gc | PORTCFG_VP1MAP_PORTD_gc;

	// This variable will be set by ISR to trigger adc conversion.
	read_request = 0;

	// Start timer.
	init_and_start_timer(TC_CLKSEL_DIV64_gc, 21); // minimum: 169

	// khz = N[2*14745600/64/21]
	// 21942.9
	// baud = N[khz*(17/4)*8]
	// 746057.

	// Enable global interrupts.
	sei();
	// High, Medium and Low Level Interrupt Enable
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;

//	PORTD.OUTSET = PIN0_bm;
	while (1) {
		PORTD.OUTTGL = PIN0_bm;
//		PORTD.OUTCLR = PIN0_bm;
		while (!read_request) {};
//		if (TCC1.CNT >= TCC0.PER) {
//			PORTD.OUTSET = PIN7_bm;
//		}
//		while (TCC0.CNT>2) {};
//		PORTD.OUTSET = PIN0_bm;
//		TCC1.CNT = 0;
		get_adc_values();
		//send_as_voltage();
		send_as_single_byte_sequence();

		read_request = 0;
	}

	return 0;
}

/*
 * ISR for Timer. This will trigger ADC conversion and USART transmission.
 */
ISR(TCC0_OVF_vect){
	if (read_request) {
		PORTD.OUTSET = PIN7_bm;
	} else {
		read_request = 1;
	}
}

/*
 * ISR for USART. Transmit byte, if buffer not empty and USART ready.
 */
//ISR(USARTF0_DRE_vect)
//{
//	if (usart_buffer_bytes > 0) {
//		USART.DATA = usart_buffer[--usart_buffer_bytes];
//	} else {
//		// disable interrupt
//		USART.CTRLA |= (USART.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
//	}
//}
