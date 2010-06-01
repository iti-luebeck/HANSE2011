#ifndef _TWISLAVE_H
#define _TWISLAVE_H

#include <util/twi.h> 		  //enthaelt z.B. die Bezeichnungen fuer die Statuscodes in TWSR
#include <avr/interrupt.h>  //dient zur behandlung der Interrupts
#include <stdint.h> 		    //definiert den Datentyp uint8_t

#include "parameter.h"


/**@brief Initaliserung des TWI-Inteface. Muss zu Beginn aufgerufen werden, sowie bei einem Wechsel der Slave Adresse
 * @param adr gewuenschte Slave-Adresse */
void init_twi_slave(uint8_t adr);

//%%%%%%%% ab hier sind normalerweise keine weiteren Aenderungen erforderlich! %%%%%%%%//
//____________________________________________________________________________________//


//Bei zu alten AVR-GCC-Versionen werden die Interrupts anders genutzt, daher in diesem Fall mit Fehlermeldung abbrechen
#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
	#error "This library requires AVR-GCC 3.4.5 or later, update to newer AVR-GCC compiler !"
#endif

//Schutz vor unsinnigen Buffergroessen
#if (i2c_buffer_size > 254)
	#error Buffer zu gross gewaehlt! Maximal 254 Bytes erlaubt.
#endif

#if (i2c_buffer_size < 2)
	#error Buffer muss mindestens zwei Byte gross sein!
#endif

/*@}*/

#endif //#ifdef _TWISLAVE_H
////Ende von twislave.h////
