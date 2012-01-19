#include <stdint.h>

#include "efm32.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"
#include "efm32_usart.h"

#include "radio.h"

uint8_t Radio_read_reg(uint8_t loc);
void Radio_write_reg(uint8_t loc, uint8_t val);
void Radio_write_lreg(uint8_t loc, uint8_t val[5]);

#define RADIO_USART    USART1
#define RADIO_LOCATION 1
#define RADIO_PORT     gpioPortD
#define RADIO_PIN_TX   0
#define RADIO_PIN_RX   1
#define RADIO_PIN_CLK  2
#define RADIO_PIN_CS   3
#define RADIO_PIN_CE   4
#define RADIO_PIN_IRQ  6

/* Defines */
#define HFRCO_FREQUENCY         14000000
#define SPI_PERCLK_FREQUENCY    HFRCO_FREQUENCY
#define SPI_BAUDRATE            1000000

#define RADIO_SPI_NUMBER   1
#define RADIO_SPI_LOCATION 1

#define RADIO_CMD_R_REG 0x00
#define RADIO_CMD_W_REG 0x20

#define RADIO_RX_ADDR_P0 0x0A
#define RADIO_TX_ADDR    0x10
#define RADIO_RX_PW_P1   0x11

uint8_t lastStatus = 0;

/**************************************************************************//**
 * @brief Setup a USART as SPI
 *****************************************************************************/
void Radio_init(void)
{
	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Enable USART1 clock
	CMU_ClockEnable(cmuClock_USART1, true);

	/* Config the USART ///////////////////////////////////////////////////// */
	// Setting baudrate
	RADIO_USART->CLKDIV = 128 * (SPI_PERCLK_FREQUENCY / SPI_BAUDRATE - 2);

	// Use synchronous (SPI) mode
	RADIO_USART->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF;
	// Clear old transfers/receptions, and disabling interrupts
	RADIO_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
	RADIO_USART->IEN = 0;
	// Enable pins and setting location
	RADIO_USART->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
			| USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | (RADIO_LOCATION << 8);

	// Set to master and to control the CS line
	// Enabling Master, TX and RX
	RADIO_USART->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
	RADIO_USART->CTRL |= USART_CTRL_AUTOCS;

	// Clear previous interrupts
	RADIO_USART->IFC = _USART_IFC_MASK;

	/* Config the IO pins /////////////////////////////////////////////////// */
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_TX, gpioModePushPull, 0); /* MOSI */
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_RX, gpioModeInput, 0); /* MISO */
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_CS, gpioModePushPull, 0); /* CS */
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_CLK, gpioModePushPull, 0); /* Clock */

	/* Config Radio ///////////////////////////////////////////////////// */
	// TODO
	uint8_t addr0[5];
	addr0[0] = 0x00;
	addr0[1] = (DEVINFO->UNIQUEL >> 0)  & 0xff;
	addr0[2] = (DEVINFO->UNIQUEL >> 8)  & 0xff;
	addr0[3] = (DEVINFO->UNIQUEL >> 16) & 0xff;
	addr0[4] = (DEVINFO->UNIQUEL >> 24) & 0xff;
	Radio_write_lreg(RADIO_RX_ADDR_P0, addr0);
	Radio_write_lreg(RADIO_TX_ADDR, addr0);
	Radio_write_reg(RADIO_RX_PW_P1, 32);

	/* Config IRQ //////////////////// */
	// GPIO_IntConfig(RADIO_PORT, RADIO_IRQ, false, true, true);
	// NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	// NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void USART_Rx_Clear(USART_TypeDef * usart)
{
	// Make sure compiler keeps it in and stops it warning
	volatile uint8_t buf __attribute__((unused));

	while ((usart->STATUS & USART_STATUS_RXDATAV)) {
		buf = usart->RXDATA;
	}
}

uint8_t Radio_read_reg(uint8_t loc)
{
	uint16_t buf;

	EFM_ASSERT(loc < 128);

	// clear RX buffer
	USART_Rx_Clear(USART1);
	// send cmd and NOP
	USART_TxDouble(USART1, (0xff << 16) | RADIO_CMD_R_REG | loc);
	// read both status and cmd
	buf = USART_RxDouble(USART1);
	// save status
	lastStatus = buf & 0xff;

	// return cmd
	return ((buf >> 8) & 0xff);
}

void Radio_write_reg(uint8_t loc, uint8_t val)
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	USART_Rx_Clear(USART1);
	// send cmd and NOP
	USART_TxDouble(USART1, (0xff << val) | RADIO_CMD_W_REG | loc);
	// read both status and cmd
	lastStatus = USART_Rx(USART1);
	// clear RX buffer
	USART_Rx_Clear(USART1);
}

void Radio_write_lreg(uint8_t loc, uint8_t val[5])
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	USART_Rx_Clear(USART1);
	// send cmd and NOP
	USART_TxDouble(USART1, (0xff << val[0]) | RADIO_CMD_W_REG | loc);
	USART_TxDouble(USART1, (0xff << val[1]) | val[2]);
	USART_TxDouble(USART1, (0xff << val[3]) | val[4]);
	// read both status and cmd
	lastStatus = USART_Rx(USART1);
	// clear RX buffer
	USART_Rx_Clear(USART1);
}

int Radio_Send(uint8_t* data, uint8_t start, uint8_t length)
{
	EFM_ASSERT(length < 32);

	// TODO

	return -1;
}
