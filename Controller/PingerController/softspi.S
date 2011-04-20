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

// je 2 Byte im RAM reservieren
.comm adc_res_0, 2
.comm adc_res_1, 2
.comm adc_res_2, 2
.comm adc_res_3, 2

.global get_adc_values
.func get_adc_values
get_adc_values:
  push workreg1 // for loop
  push workreg2 // for input byte
  push workreg3 // adc0
  push workreg4 // adc1
  push workreg5 // adc2
  push workreg6 // adc3

	// Start conversion.
  cbi _SFR_IO_ADDR(VPORT0_OUT), PIN3_bp // cs low, 1 cyle

  // Conversion needs two cycles to get ready. In the meanwhile, prepare some registers to save time.
  cbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk low, 1 cyle
  nop // 1 cycle
  nop // 1 cycle
  nop // 1 cycle
  sbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk high, 1 cyle
  nop // 1 cycle
  nop // 1 cycle
  nop // 1 cycle
  cbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk low, 1 cyle
  nop // 1 cycle
	ldi workreg1, 0x0E // 1 cycle; data acquisition loop has to run 14 times (0x0E), so prepare a register
	clr workreg3 // 1 cycle; clear adc tmp register
  sbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk high, 1 cyle
	clr workreg4 // 1 cycle; clear adc tmp register
	clr workreg5 // 1 cycle; clear adc tmp register
	clr workreg6 // 1 cycle; clear adc tmp register

	read_loop:
	  // Falling flank. First/next data bit will appear and can be captured right before rising flank.
		// First loop: In time!
		// Sixth loop: 13 cycles late
		// All other 12 loops: 2 cycles late
  	cbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk low, 1 cyle

	  // Left shift for adc tmp registers to get space for new bit.
  	lsl workreg3 // 1 cycle
  	lsl workreg4 // 1 cycle
  	lsl workreg5 // 1 cycle
  	lsl workreg6 // 1 cycle

  	// Read 4 bits and store them in the corresponding registers.
  	// Hint: Reading the input right after the low flank is also possible, because the chip needs at most 40ns to make data appear.
  	in workreg2, _SFR_IO_ADDR(VPORT0_IN) // 1 cycle, read data from multiple MISO pins
	  bst workreg2, 6 // 1 cycle
	  bld workreg3, 0 // 1 cycle
	  bst workreg2, 1 // 1 cycle
	  bld workreg4, 0 // 1 cycle
	  bst workreg2, 2 // 1 cycle
	  bld workreg5, 0 // 1 cycle
	  bst workreg2, 0 // 1 cycle
	  bld workreg6, 0 // 1 cycle

		// Rising flank.
		// Always 10 cycles late.
	  sbi _SFR_IO_ADDR(VPORT0_OUT), PIN7_bp // sclk high, 1 cycle

		// After 6 bits, store high byte.
		// This block: 2 cycles on false, 13 cycles on true.
		cpi workreg1, 0x09 // 1 cycle
		breq store_high_byte // 1 cycle if false, 2 cycles if true
		ret_from_store_high_byte:

		// Jump to next loop run or quit after 14 loops.
  	dec workreg1 // 1 cycle
		brbc 1,read_loop // 1 cycle if false, 2 cycles if true

	// store low byte
	sts adc_res_0, workreg3
	sts adc_res_1, workreg4
	sts adc_res_2, workreg5
	sts adc_res_3, workreg6

	// Enter acquisition mode after completed transmission.
  // Attention: The chip needs at least two cycles for acquisition until CS can be brought up again.
  sbi _SFR_IO_ADDR(VPORT0_OUT), PIN3_bp // cs high

  pop workreg6
  pop workreg5
  pop workreg4
  pop workreg3
  pop workreg2
  pop workreg1
  ret

store_high_byte:
	sts adc_res_0+1, workreg3 // 2 cycles
	sts adc_res_1+1, workreg4 // 2 cycles
	sts adc_res_2+1, workreg5 // 2 cycles
	sts adc_res_3+1, workreg6 // 2 cycles
  rjmp ret_from_store_high_byte // 2 cycles

.end
