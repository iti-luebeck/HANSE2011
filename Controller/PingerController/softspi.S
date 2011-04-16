/*
 * softspi.S
 *
 *  Created on: 13.04.2011
 *      Author: martin
 */

#include "avr/io.h"

//; Arbeitsregister (ohne "r")
workreg  = 16
workreg2 = 17
workreg3 = 18
workreg4 = 19

//; ** Setze alle Pins von PortD auf Ausgang **
//; keine Parameter, keine Rückgabe
.global ledOff
.func ledOff
ledOff:
   push workreg
   ldi workreg, PIN0_bm
   out  _SFR_IO_ADDR(VPORT0_OUT), workreg  // beachte: _SFR_IO_ADDR()
   pop workreg
   ret
.endfunc

.global ledOn
.func ledOn
ledOn:
   push workreg
   ldi workreg, 0x00
   out  _SFR_IO_ADDR(VPORT0_OUT), workreg  // beachte: _SFR_IO_ADDR()
   pop workreg
   ret
.endfunc

.global readADC
.func readADC
readADC:
  push workreg
  push workreg2
  push workreg3
  push workreg4

  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN3_bp // cs low
  rcall some_sleep

  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep
  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep
  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep
  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep
  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep

	ldi workreg3, 0x0E
	clr workreg
read_loop:
  lsl workreg // shift 16bit result
  rol workreg2
  in workreg4, _SFR_IO_ADDR(VPORT1_IN) // read bit from MISO
  and workreg4, 24
  brbs 1, lowBit
  //sbic _SFR_IO_ADDR(VPORT1_IN), PIN6_bp
  sbr workreg, 0x01
  lowBit:

  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep
  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep

  dec workreg3
	brbc 1,read_loop

  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep
  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN3_bp // cs high
  nop
  nop

  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep
  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep
  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low
  rcall some_sleep
  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  rcall some_sleep

  //ldi workreg, 0x00
  //ldi workreg2, 0xFF
	movw r24, workreg

  pop workreg4
  pop workreg3
  pop workreg2
  pop workreg
  ret
.endfunc

some_sleep:
  nop
  nop
  nop
  nop
  nop
  ret

.end
