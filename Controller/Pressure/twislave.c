#include <util/twi.h> 		//enth�lt z.B. die Bezeichnungen f�r die Statuscodes in TWSR
#include <avr/interrupt.h>  //dient zur Behandlung der Interrupts
#include <stdint.h> 		//definiert den Datentyp uint8_t
#include "twislave.h"
#include "parameter.h"

#include "spi.h"

volatile uint8_t calib = 0;
volatile uint8_t i2cdata[i2c_buffer_size+1];

/*
Aufbau i2cdata:
0-7:	Calibration Words
8-9:	Pressure (RAW)
10-11:	Temperature (RAW)
12-13:	Pressure
14-15:	Temperature
16:		TWI-Errorcode
17:		0x55
18:		Calib
//19:		Starte Aulesen Kalibrierung
20:		I2C-Counter
*/

volatile uint8_t buffer_adr; //"Adressregister" f�r den Buffer


void init_twi_slave(uint8_t adr)
{
	TWAR= adr; //Adresse setzen
	TWCR &= ~(1<<TWSTA)|(1<<TWSTO);
	TWCR|= (1<<TWEA) | (1<<TWEN)|(1<<TWIE); 	
	buffer_adr=0xFF;
	i2cdata[16]=0x00;
	i2cdata[17]=0x55;
	i2cdata[20]=0x00;  
	sei();

}



//ACK nach empfangenen Daten senden/ ACK nach gesendeten Daten erwarten
#define TWCR_ACK TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);  

//NACK nach empfangenen Daten senden/ NACK nach gesendeten Daten erwarten     
#define TWCR_NACK TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);

//switched to the non adressed slave mode...
#define TWCR_RESET TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);  
//Die Bitmuster f�r TWCR_ACK und TWCR_RESET sind gleich. Dies ist kein Fehler und dient nur der �bersicht!

/*ISR, die bei einem Ereignis auf dem Bus ausgel�st wird. Im Register TWSR befindet sich dann 
ein Statuscode, anhand dessen die Situation festgestellt werden kann.
*/
ISR (TWI_vect) {
	uint8_t data=0;


	switch (TW_STATUS) {//TWI-Statusregister pr�fen und n�tige Aktion bestimmen 

		// Slave Receiver 

		case TW_SR_SLA_ACK: // 0x60 Slave Receiver, Slave wurde adressiert	
			TWCR_ACK; // n�chstes Datenbyte empfangen, ACK danach senden
			buffer_adr=0xFF; //Bufferposition ist undefiniert
			break;
	
		case TW_SR_DATA_ACK: // 0x80 Slave Receiver, ein Datenbyte wurde empfangen
			data=TWDR; //Empfangene Daten auslesen
			if (buffer_adr == 0xFF) //erster Zugriff, Bufferposition setzen
				{
					//Kontrolle ob gew�nschte Adresse im erlaubten bereich
					if(data<i2c_buffer_size)
						{
							buffer_adr= data; //Bufferposition wie adressiert setzen
						}
					else
						{
							buffer_adr=0;
						}
					if (data==18) calib=1;				
					TWCR_ACK;	// n�chstes Datenbyte empfangen, ACK danach, um n�chstes Byte anzufordern
				}
			else //weiterer Zugriff, nachdem die Position im Buffer gesetzt wurde. NUn die Daten empfangen und speichern
				{
		
					if(buffer_adr<i2c_buffer_size)
						{
							if(!I2C_reg_Schreibschutz[buffer_adr]) //Wenn Position nicht schreibgesch�tzt...
								i2cdata[buffer_adr]=data; 			//...dann Daten in Buffer schreibe
						
						
						}
					buffer_adr++; //Buffer-Adresse weiterz�hlen f�r n�chsten Schreibzugriff
					TWCR_ACK;	
				}
		break;


		//Slave transmitter

		case TW_ST_SLA_ACK: //0xA8 Slave wurde im Lesemodus adressiert und hat ein ACK zur�ckgegeben.
			//Hier steht kein break! Es wird also der folgende Code ebenfalls ausgef�hrt!
	
		case TW_ST_DATA_ACK: //0xB8 Slave Transmitter, Daten wurden angefordert

			if (buffer_adr == 0xFF) //zuvor keine Leseadresse angegeben! 
				{
					buffer_adr=0;
				}	
		
			if(buffer_adr<i2c_buffer_size)	
				{
					TWDR = i2cdata[buffer_adr]; //Datenbyte senden
					buffer_adr++; //bufferadresse f�r n�chstes Byte weiterz�hlen
				}
			else
				{
					TWDR=0; //Kein Daten mehr im Buffer
				}
			TWCR_ACK;
		break;

		case TW_BUS_ERROR:
			i2cdata[16]=TW_STATUS;
			// bus error due to an illegal start/stop condition
			// see avr168 datasheet, section 21.7.5
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC);
			break;
		case TW_ST_DATA_NACK: // 0xC0 Keine Daten mehr gefordert 
			i2cdata[16]=TW_STATUS; 
		case TW_SR_DATA_NACK: // 0x88
			i2cdata[16]=TW_STATUS;  
		case TW_ST_LAST_DATA: // 0xC8  Last data byte in TWDR has been transmitted (TWEA = �0�); ACK has been received
			i2cdata[16]=TW_STATUS; 
		default:
			i2cdata[16]=TW_STATUS; 	
		    TWCR_RESET;
		break;
	
	}
}

