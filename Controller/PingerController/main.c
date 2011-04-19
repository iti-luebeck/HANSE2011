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

extern void ledOff();
extern void ledOn();
extern void readADC();

extern uint16_t adc_res_0, adc_res_1, adc_res_2, adc_res_3;

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

	// PLL source XOSC, factor 2
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (2 << OSC_PLLFAC_gp);

	// Enable PLL
	OSC.CTRL |= OSC_PLLEN_bm;

	// wait for PLL to get ready
	while( (OSC.STATUS & OSC_PLLRDY_bm) == 0) {
//	  PORTD.OUTTGL = PIN0_bm;
	}

	// CLK.CTRL is protected, so we have to allow modification by the following command.
	CCP = CCP_IOREG_gc;
	// SCLKSEL is used to select the source for the System Clock.
	//CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;
}

#define USART USARTF0


void send_char(char c) {
	while(!USART_IsTXDataRegisterEmpty(&USART)) {};
	USART_PutChar(&USART, c);
}

void send_as_voltage(uint16_t* adcs[]) {
	for (int p=0; p<4; p++) {
		// set chip active
//		PORTC.OUTCLR = PIN3_bm;

//		// transreceive byte
//		SPIC.DATA = 0;
//		while (!(SPIC.STATUS & SPI_IF_bm)) {};
//		receivedData1 = SPIC.DATA;
//		SPIC.DATA = 0;
//		while (!(SPIC.STATUS & SPI_IF_bm)) {};
//		receivedData2 = SPIC.DATA;
//		// set chip back to acquisition mode
//		PORTC.OUTSET = PIN3_bm;
//		SPIC.DATA = 0;
//		while (!(SPIC.STATUS & SPI_IF_bm)) {};

		int16_t a;
		a = (*adcs[p] << 2);
//		a = (receivedData1 << 10) | (receivedData2 << 2);
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

//		for (int i=0; i<8; i++) {
//			while(!USART_IsTXDataRegisterEmpty(&USART)) {};
//			tmp8 = receivedData1 << i;
//			tmp8 = tmp8 >> 7;
//			USART_PutChar(&USART, tmp8 + '0');
//		}
//		for (int i=0; i<8; i++) {
//			while(!USART_IsTXDataRegisterEmpty(&USART)) {};
////			USART_PutChar(&USART, 0x39);
//			USART_PutChar(&USART, (((receivedData2<<i) & 0x80)>>7) + '0');
//		}
		if (p==3) {
			send_char('\r');
		} else {
			send_char(';');
		}
	}
}

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
	int16_t result;

  	// PC3 (TXD0) as output.
	PORTF.DIRSET   = PIN3_bm;
	// PC2 (RXD0) as input.
	PORTF.DIRCLR   = PIN2_bm;

	long baud = 9600;
	int8_t bScale = 0;
	uint16_t baud_setting = F_CPU / 16 / baud - 1;
    USART.BAUDCTRLA = (uint8_t)baud_setting;
    USART.BAUDCTRLB = (bScale << USART_BSCALE0_bp) | (baud_setting >> 8);

    // enable Tx
	USART.CTRLB |= USART_TXEN_bm;

	// Char size, parity and stop bits: 8N1
	USART.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;

	while (1) {
		while ( !(USART.STATUS & USART_DREIF_bm) );
		USART.DATA = 'x';
	}

	PORTCFG.VPCTRLA=PORTCFG_VP0MAP_PORTD_gc;
	PORTCFG.VPCTRLA=PORTCFG_VP1MAP_PORTC_gc;
//	while (1) {
//		_delay_ms(1000);
//		ledOff();
//		_delay_ms(1000);
//		ledOn();
//	}

	/* USARTC0, 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(&USART, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

	/* Set Baudrate to 9600 bps:
	 * Do not use the baudrate scale factor
	 *
	 * Baudrate select = (I/O clock frequency)/(16 * Baudrate) - 1
	 *                 16000000/(16*9600)-1 = 103
	 */
	//4*6+1 = 25 byte/timestep (8 byte data)
	//40kHz => 40.000*25*8=8G
	// 921600 baud
	//USART_Baudrate_Set(&USART, 103 , 0);
	USART_Baudrate_Set(&USART, 1110, -7);
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

	// define MISO as input
	PORTC.DIRCLR = PIN6_bm;
	PORTC.DIRCLR = PIN0_bm;
	PORTC.DIRCLR = PIN1_bm;
	PORTC.DIRCLR = PIN2_bm;

	// define CS as output
	PORTC.DIRSET = PIN3_bm;
	PORTC.OUTSET = PIN3_bm;

	// initialize SPI master
//	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_3_gc | SPI_PRESCALER_DIV16_gc;

	uint16_t* adcs[] = {&adc_res_0, &adc_res_1, &adc_res_2, &adc_res_3};

	PORTD.OUT &= ~PIN0_bm;
	while (1) {
		PORTD.OUTTGL = PIN0_bm;
		// receive data
		readADC();
		send_as_voltage(adcs);

//		uint16_t timeout = 1000;
		/* Receive one char. */
//		do{
//		/* Wait until data received or a timeout.*/
//		timeout--;
//		}while(!USART_IsRXComplete(&USART) && timeout!=0);
////		receivedData = USART_GetChar(&USART);
//		if (timeout > 0) {
//			PORTD.OUTTGL = PIN0_bm;
//		}

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
