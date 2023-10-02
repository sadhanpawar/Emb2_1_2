/*
 * spi0.h
 *
 *  Created on: Jan 27, 2023
 *      Author: sadhan
 */

#ifndef SPI0_H_
#define SPI0_H_

#define USE_SSI_FSS 3
#define USE_SSI_RX  4

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initSpi0(uint32_t pinMask);
void setSpi0BaudRate(uint32_t clockRate, uint32_t fcyc);
void setSpi0Mode(uint8_t polarity, uint8_t phase);
void writeSpi0Data(uint32_t data);
uint32_t readSpi0Data();



#endif /* SPI0_H_ */
