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
	uint8_t receivedData;

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


	/* Assume that everything is OK. */
//	success = true;
	/* Send data from 255 down to 0*/
	sendData = 255;
	while(sendData) {
	    /* Send one char. */
		do{
		/* Wait until it is possible to put data into TX data register.
		 * NOTE: If TXDataRegister never becomes empty this will be a DEADLOCK. */
		}while(!USART_IsTXDataRegisterEmpty(&USART));
		USART_PutChar(&USART, 0x39);

		uint16_t timeout = 1000;
		/* Receive one char. */
		do{
		/* Wait until data received or a timeout.*/
		timeout--;
		}while(!USART_IsRXComplete(&USART) && timeout!=0);
		receivedData = USART_GetChar(&USART);
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

//	/* Use USARTF0 and initialize buffers. */
//	USART_InterruptDriver_Initialize(&USART_data, &USARTF0, USART_DREINTLVL_LO_gc);
//
//	/* USARTF0, 8 Data bits, No Parity, 1 Stop bit. */
//	USART_Format_Set(USART_data.usart, USART_CHSIZE_8BIT_gc,
//                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	//USART_RxdInterruptLevel_Set(USART_data.usart, USART_RXCINTLVL_LO_gc);

	/* Set Baudrate to 9600 bps:
	 * Do not use the baudrate scale factor
	 *
	 * Baudrate select = (I/O clock frequency)/(16 * Baudrate) - 1
	 *                 16000000/(16*9600)-1 = 103
	 */
//	USART_Baudrate_Set(&USARTF0, 103 , 0);
////	USART_Baudrate_Set(&USARTF0, 12 , 0);
//
//	/* Enable both RX and TX. */
//	//USART_Rx_Enable(USART_data.usart);
//	USART_Tx_Enable(USART_data.usart);
//
//	/* Enable PMIC interrupt level low. */
//	PMIC.CTRL |= PMIC_LOLVLEX_bm;
//
//	/* Enable global interrupts. */
//	sei();
//
//	/* counter variable. */
//	uint8_t i;

	/* Send sendArray. */

	while (1) {
		_delay_ms(1000);
		PORTD.OUT |= PIN0_bm;
		_delay_ms(1000);
		PORTD.OUT &= ~PIN0_bm;
//		i = 0;
//		while (i < NUM_BYTES) {
//			bool byteToBuffer;
//			byteToBuffer = USART_TXBuffer_PutByte(&USART_data, sendArray[i]);
//			if(byteToBuffer){
//				i++;
//			}
//		}
	}
	return 0;
}
