#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <util/atomic.h>


#include "Pinger.h"
#include "arduino/WProgram.h"

Pinger pinger;
uint16_t mic[3];
int temp = 3;
int cnt = 0;


void loop()
{



	if (temp < 3){
		Serial.println("-------");
		Serial.println(temp);
		Serial.println("-------");
		temp=3;
	}


   	if (cnt==3) { 													//wenn alle signale erhalten
   		for(int i=0; i<=2; i++) {
   			Serial.println(mic[i]);									//sende timestamps über die serielle schnittstelle
   			mic[i]=0;												//und setze timestamps auf 0
   		}
   		Serial.println("-------");
   		TCCR1B &= ~(1 << CS10);										//timer aus
   		TCNT1 = 0x0000;												//reset timer
   		cnt=0;

   		_delay_ms(500);
   		EIFR = (1 << INTF0) | (1 << INTF1) | (1 << INTF2);
   		EIMSK |= ((1 << INT0) | (1 << INT1) | (1 << INT2));			//interrupts wieder aktivieren
   	}


//	_delay_ms(500);					//blinkende LED
//    PORTD |= (1 << PD7);
//
//	_delay_ms(500);
//    PORTD &= (0 << PD7);

}

void setup() {


    Serial.begin(9600);
    TCCR1B &= ~((1 << CS10) | (1 << CS11));		//korrigiere blöden mist
    TCCR1A &= ~(1 << WGM10);					//aus dem datenblatt
    TCNT1 = 0x0000;
    sei();
    DDRD |= (1 << PD7);
    EICRA |= ((1 << ISC00) | (1 << ISC01) | (1 << ISC10) | (1 << ISC11) | (1 << ISC20) | (1 << ISC21)); //Interrupt bei steigender Flanke
    EIMSK |= ((1 << INT0) | (1 << INT1) | (1 << INT2)); //enable external interrupts 0-2
	for(int i=0; i<=2; i++) {
		mic[i]=0;	//initialisiere timestamps
	}
}

ISR(INT0_vect)							//external interrupt 0
{
	EIMSK &= ~(1 << INT0);				//disable interrupt
	mic[0] = TCNT1;						//save timestamp
	TCCR1B |= (1 << CS10);				//timer an
	temp = 0;
	cnt++;
}

ISR(INT1_vect)							//external interrupt 1
{
	EIMSK &= ~(1 << INT1);				//disable interrupt
	mic[1] = TCNT1;						//save timestamp
	TCCR1B |= (1 << CS10);				//timer an
	temp = 1;
	cnt++;
}

ISR(INT2_vect)							//external interrupt 2
{
	EIMSK &= ~(1 << INT2);				//disable interrupt
	mic[2] = TCNT1;						//save timestamp
	TCCR1B |= (1 << CS10);				//timer an
	temp = 2;
	cnt++;
}


int main() {

    init();

    setup();

    while (true)
    	loop();

    return 0;
}