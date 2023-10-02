// Stop Go C Example (Basic)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// Pushbutton:
//   SW1 pulls pin PF4 low (internal pull-up is used)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "spi0.h"
#include "wait.h"
#include "stop_go.h"
#include "nvic.h"

// Pins
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4
#define SPI0_INT PORTB, 6

#define SW_CTRL_SSI0FSS PORTA,3
#define SPI_MCP_2MHZ_CLK (2e6)

#define WRITE_TO_SPI0   true
#define READ_FROM_SPI0  false


/* Bank 0 */
static uint8_t mcp23s08Configuration[][3] = {
                                        {0x46, 0x05,0x02}, /* IOCON: 0x00110010 */
                                        {0x46, 0x00,0x10}, /* IODIRA 5th pin Ip, 0-3 Ops*/
                                        {0x46, 0x01,0x00}, /* IPOLA  same logic */
                                        {0x46, 0x06,0x10}, /* GPPU: pull ups for inputs */
                                        //{0x46, 0x02,0x10}, /* GPINTEN enabled for  inputs */
                                        {0x46, 0x03,0x10}, /* DEFVAL: enabled for inputs(pull down) */
                                        {0x46, 0x04,0x10}, /* INTCON: */

};

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


// Blocking function that returns only when SW1 is pressed
void waitPbPress()
{
    uint32_t data = 0;

    while( true ) /*1st bit */
    {
        talkToSpiDev(0x47,0x09,&data,READ_FROM_SPI0);

        data = (data & 0xF0);
        if(data == false ) {
            break;
        }
    }
}

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTF);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinDigitalInput(PUSH_BUTTON);
    enablePinPullup(PUSH_BUTTON);
    

    /* enable clocks */
    enablePort(PORTB);
    selectPinDigitalInput(SPI0_INT);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    uint32_t data = 0;
    uint32_t intc_data = 0xFF;
    uint32_t int_data = 0;
    uint8_t count = 10;

	// Initialize hardware
	initHw();

	configSpi0Mcp23s08();

    data = 0x01;
	talkToSpiDev(0x46,0x09,&data,WRITE_TO_SPI0);

	/* wait till push button is pressed */
	waitPbPress();

	data = 0x02;
	talkToSpiDev(0x46,0x09,&data,WRITE_TO_SPI0);

	/*clear the interrupt in spi slave dev */
	while( count > 0 ) {
	    talkToSpiDev(0x47,0x09,&intc_data,READ_FROM_SPI0);
	    count--;
	}

	waitMicrosecond(1000000);

    clearPinInterrupt(SPI0_INT);

    /* enable the nvic gpio port B interrupt */
    enableNvicInterrupt(INT_GPIOB);
    enablePinInterrupt(SPI0_INT);
    selectPinInterruptRisingEdge(SPI0_INT);

    /* enable interrupt in slave device */
    int_data = 0x10;
    talkToSpiDev(0x46,0x02,&int_data,WRITE_TO_SPI0);

    /*write to led */
    data = 0x01;
    talkToSpiDev(0x46,0x09,&data,WRITE_TO_SPI0);

    // Endless loop
    while(true);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void configSpi0Mcp23s08(void)
{
    /* Configure SPI0 module with SW controlled FSS */
    initSpi0(USE_SSI_RX);

    setSpi0BaudRate(SPI_MCP_2MHZ_CLK,40e6);

    setSpi0Mode(1,1);

    enablePinPullup(SW_CTRL_SSI0FSS);

    setPinValue(SW_CTRL_SSI0FSS,true);

    writeSpi0RegData();

}

void enableSpi0Fss(void)
{
    setPinValue(SW_CTRL_SSI0FSS,false);
    _delay_cycles(4);
}

void disableSpi0Fss(void)
{
    setPinValue(SW_CTRL_SSI0FSS,true);
}

void writeSpi0RegData(void)
{
    
    uint8_t i;
    uint8_t j;
    uint32_t intc_data = 0;

    for( i = 0; i < sizeof(mcp23s08Configuration)/sizeof(mcp23s08Configuration[0]) ;i++ )
    {
        enableSpi0Fss();
        for(j = 0; j < sizeof(mcp23s08Configuration[i]); j++)
        {

            writeSpi0Data(mcp23s08Configuration[i][j]);

            (void)readSpi0Data();

        }
        disableSpi0Fss();
        waitMicrosecond(100000);
    }

    /*clear the interrupts if there are any */
    talkToSpiDev(0x47,0x09,&intc_data,READ_FROM_SPI0);
}

void talkToSpiDev(uint8_t opcode, uint8_t reg, uint32_t *data, bool flag)
{
    waitMicrosecond(10000);

    enableSpi0Fss();

    writeSpi0Data(opcode);

    (void)readSpi0Data();

    writeSpi0Data(reg);

    (void)readSpi0Data();

    if (flag) {
        writeSpi0Data(*data);

        (void)readSpi0Data();
    } else {
        writeSpi0Data(0);

        *data = readSpi0Data();
    }

    disableSpi0Fss();

    waitMicrosecond(10000);
}

void mcpGpioPortBIntHandler(void)
{
    uint32_t data;

    /*clear the interrupt in spi slave dev */
    talkToSpiDev(0x47,0x09,&data,READ_FROM_SPI0);

    /* toggle the led */
    data = 0x02;
    talkToSpiDev(0x46,0x09,&data,WRITE_TO_SPI0);

    clearPinInterrupt(SPI0_INT);
}
