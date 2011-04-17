/*
 * softspi.S
 *
 *  Created on: 13.04.2011
 *      Author: martin
 */

#include "avr/io.h"

//; Arbeitsregister (ohne "r")
workreg1 = 16
workreg2 = 17
workreg3 = 18
workreg4 = 19
workreg5 = 20
workreg6 = 21

//; ** Setze alle Pins von PortD auf Ausgang **
//; keine Parameter, keine Rückgabe
.global ledOff
.func ledOff
ledOff:
   push workreg1
   ldi workreg1, PIN0_bm
   out  _SFR_IO_ADDR(VPORT0_OUT), workreg1  // beachte: _SFR_IO_ADDR()
   pop workreg1
   ret
.endfunc

.global ledOn
.func ledOn
ledOn:
   push workreg1
   ldi workreg1, 0x00
   out  _SFR_IO_ADDR(VPORT0_OUT), workreg1  // beachte: _SFR_IO_ADDR()
   pop workreg1
   ret
.endfunc

// je 2 Byte im RAM reservieren
.comm adc_res_0, 2
.comm adc_res_1, 2
.comm adc_res_2, 2
.comm adc_res_3, 2

.global readADC
.func readADC
readADC:
  push workreg1 // for loop
  push workreg2 // for input byte
  push workreg3 // adc0
  push workreg4 // adc1
  push workreg5 // adc2
  push workreg6 // adc3

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

	ldi workreg1, 0x0E
	clr workreg3
	clr workreg4
	clr workreg5
	clr workreg6
read_loop:
  lsl workreg3
  lsl workreg4
  lsl workreg5
  lsl workreg6

  in workreg2, _SFR_IO_ADDR(VPORT1_IN) // read data from multiple MISO pins

  //and workreg4, 24
  //brbs 1, lowBit
  //sbic _SFR_IO_ADDR(VPORT1_IN), PIN6_bp
  //sbr workreg3, 0x01
  //lowBit:

  sbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk high
  bst workreg2, 6 // 1 cycle
  bld workreg3, 0 // 1 cycle
  bst workreg2, 1 // 1 cycle
  bld workreg4, 0 // 1 cycle
  bst workreg2, 2 // 1 cycle
  bld workreg5, 0 // 1 cycle
  bst workreg2, 0 // 1 cycle
  bld workreg6, 0 // 1 cycle

  cbi _SFR_IO_ADDR(VPORT1_OUT), PIN7_bp // sclk low

	cpi workreg1, 0x09
	breq store_high_byte
	ret_from_store_high_byte:

  dec workreg1
	brbc 1,read_loop

	// store low byte
	sts adc_res_0, workreg3
	sts adc_res_1, workreg4
	sts adc_res_2, workreg5
	sts adc_res_3, workreg6

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
	//movw r24, workreg

  pop workreg6
  pop workreg5
  pop workreg4
  pop workreg3
  pop workreg2
  pop workreg1
  ret
.endfunc

store_high_byte:
	sts adc_res_0+1, workreg3
	sts adc_res_1+1, workreg4
	sts adc_res_2+1, workreg5
	sts adc_res_3+1, workreg6
  rjmp ret_from_store_high_byte

some_sleep:
  ret

.end
