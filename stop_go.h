/*
 * stop_go.h
 *
 *  Created on: Jan 27, 2023
 *      Author: sadhan
 */

#ifndef STOP_GO_H_
#define STOP_GO_H_


extern void configSpi0Mcp23s08(void);
extern void writeSpi0RegData(void);
extern void mcpGpioPortBIntHandler(void);
extern void talkToSpiDev(uint8_t opcode, uint8_t reg, uint32_t *data, bool flag);
extern void writeSpi0RegData(void);

#endif /* STOP_GO_H_ */
