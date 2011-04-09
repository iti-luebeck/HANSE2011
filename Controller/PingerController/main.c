/*
 * main.c
 *
 *  Created on: Apr 5, 2011
 *      Author: robprak
 */
#include <avr/io.h>
#include <util/delay.h>
#include "usart_driver.h"

//#define F_CPU 16000000UL


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

#define USART USARTF0

int main() {
	PORTD.DIRSET |= PIN0_bm;
	PORTD.OUT |= PIN0_bm;

	// Enable external 16 MHz oscillator.
	set_external_oscillator();

		/* Variable used to send and receive data. */
	uint8_t sendData;
	uint8_t receivedData1;
	uint8_t receivedData2;
	uint8_t tmp8;

  	// PC3 (TXD0) as output.
	PORTF.DIRSET   = PIN3_bm;
	// PC2 (RXD0) as input.
	PORTF.DIRCLR   = PIN2_bm;


	/* USARTC0, 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(&USART, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

	/* Set Baudrate to 9600 bps:
	 * Do not use the baudrate scale factor
	 *
	 * Baudrate select = (I/O clock frequency)/(16 * Baudrate) - 1
	 *                 16000000/(16*9600)-1 = 103
	 */
	USART_Baudrate_Set(&USART, 103 , 0);

	/* Enable both RX and TX. */
	USART_Rx_Enable(&USART);
	USART_Tx_Enable(&USART);

	/* SPI */
	// define !SS as output
	PORTC.DIRSET = PIN4_bm;
	// define MOSI as output
	PORTC.DIRSET = PIN5_bm;
	// define SCK as output
	PORTC.DIRSET = PIN7_bm;

	// define CS as output
	PORTC.DIRSET = PIN3_bm;
	PORTC.OUTSET = PIN3_bm;

	// initialize SPI master
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_3_gc | SPI_PRESCALER_DIV16_gc;

	PORTD.OUT &= ~PIN0_bm;
	while (1) {
		// set chip active
		PORTC.OUTCLR = PIN3_bm;
		// transreceive byte
		SPIC.DATA = 0;
		while (!(SPIC.STATUS & SPI_IF_bm)) {};
		receivedData1 = SPIC.DATA;
		SPIC.DATA = 0;
		while (!(SPIC.STATUS & SPI_IF_bm)) {};
		receivedData2 = SPIC.DATA;
		// set chip back to acquisition mode
		PORTC.OUTSET = PIN3_bm;
		SPIC.DATA = 0;
		while (!(SPIC.STATUS & SPI_IF_bm)) {};

		for (int i=0; i<8; i++) {
			while(!USART_IsTXDataRegisterEmpty(&USART)) {};
			tmp8 = receivedData1 << i;
			tmp8 = tmp8 >> 7;
			USART_PutChar(&USART, tmp8 + '0');
		}
		for (int i=0; i<8; i++) {
			while(!USART_IsTXDataRegisterEmpty(&USART)) {};
//			USART_PutChar(&USART, 0x39);
			USART_PutChar(&USART, (((receivedData2<<i) & 0x80)>>7) + '0');
		}
		while(!USART_IsTXDataRegisterEmpty(&USART)) {};
		USART_PutChar(&USART, '\r');

		uint16_t timeout = 1000;
		/* Receive one char. */
		do{
		/* Wait until data received or a timeout.*/
		timeout--;
		}while(!USART_IsRXComplete(&USART) && timeout!=0);
//		receivedData = USART_GetChar(&USART);
		if (timeout > 0) {
			PORTD.OUTTGL = PIN0_bm;
		}

		/* Check the received data. */
//		if (receivedData != sendData){
//			success = false;
//		}
//		sendData--;
		_delay_ms(300);
//		PORTD.OUT |= PIN0_bm;
//		_delay_ms(300);
//		PORTD.OUT &= ~PIN0_bm;
	}


	while (1) {
		_delay_ms(1000);
		PORTD.OUT |= PIN0_bm;
		_delay_ms(1000);
		PORTD.OUT &= ~PIN0_bm;
	}
	return 0;
}
