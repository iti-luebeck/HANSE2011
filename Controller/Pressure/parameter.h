/** 
 *	@file parameter.h  
 *   
 *  @author Dipl-Inf. Christoph Osterloh  
 *  @date 11.09.2009  
 * 
 */

#ifndef PARAMETER_H
#define PARAMETER_H

#include <avr/io.h>


#ifndef F_CPU
	#define F_CPU 8000000UL		// 14.7456MHz
#endif

#include <util/delay.h>

/** 
 * 	Baudrate für die UART-Datenübertragung
 */
#define BAUD1 115200UL
#define UART_BUFFER	1024


//%%%%%%%% von Benutzer konfigurierbare Einstellungen %%%%%%%%
/**@brief Groesse des Buffers in Byte (2..254) */
#define i2c_buffer_size 	32


//%%%%%%%% Globale Variablen, die vom Hauptprogramm genutzt werden %%%%%%%%
/**@brief Der Buffer, in dem die Daten gespeichert werden.
 * Aus Sicht des Masters laeuft der Zugrif auf den Buffer genau wie
 *  bei einem I2C-EEPROm ab.
 * Fuer den Slave ist es eine globale Variable
*/
extern volatile uint8_t i2cdata[i2c_buffer_size+1];
volatile uint8_t I2C_reg_Schreibschutz[i2c_buffer_size+1];

extern volatile uint8_t calib;
#endif
