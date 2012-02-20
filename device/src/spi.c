#include <stdint.h>
#include <stdbool.h>

#include "efm32.h"

#include "efm32_gpio.h"
#include "efm32_cmu.h"

#include "spi.h"

/*
 * TODO:
 *  * Change to use of BITBAND_Peripheral where needed.
 *  * Change to __INLINE where needed.
 *  * Improve IRQ handling.
 *  * Change to __wfi and sleeping where needed.
 */

void spi_init(void) {
	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Enable USART clock
	CMU_ClockEnable(SPI_CLOCK, true);

	/* Config the USART ///////////////////////////////////////////////////// */
	// Setting baudrate (see p193 of EFM32G ref)
	SPI_USART->CLKDIV = 256 * (SPI_PERCLK_FREQUENCY / (SPI_BAUDRATE*2) - 1);

	// Use synchronous (SPI) mode 3 with MSB first
	//SPI_USART->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF
	//		| USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLETRAILING;
	SPI_USART->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF
				| USART_CTRL_CLKPOL_IDLELOW | USART_CTRL_CLKPHA_SAMPLELEADING;
	// Clear old transfers/receptions
	spi_clear_rx();
	spi_clear_tx();
	// Disable and clear interrupts
	SPI_USART->IEN = USART_IEN_RXOF | USART_IEN_TXOF | USART_IEN_TXC;
	NVIC_ClearPendingIRQ(SPI_IRQ_RXN);
	NVIC_ClearPendingIRQ(SPI_IRQ_TXN);
	NVIC_EnableIRQ(SPI_IRQ_RXN);
	NVIC_EnableIRQ(SPI_IRQ_TXN);
	// Enable pins and setting location
	SPI_USART->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
			| USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | SPI_ROUTE_LOC;

	// Set to master and to control the CS line
	// Enabling Master, TX and RX
	SPI_USART->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
	// Disable auto cs
	SPI_USART->CTRL ^= USART_CTRL_AUTOCS;

	/* Config the IO pins /////////////////////////////////////////////////// */
	GPIO_PinModeSet(SPI_PORT, SPI_PIN_TX, gpioModePushPull, 0); // MOSI
	GPIO_PinModeSet(SPI_PORT, SPI_PIN_RX, gpioModeInput, 0); // MISO
	GPIO_PinModeSet(SPI_PORT, SPI_PIN_CLK, gpioModePushPull, 0); // Clock
	GPIO_PinModeSet(SPI_PORT, SPI_PIN_CS, gpioModePushPull, 0); // CS
}

int spi_read(uint8_t* buffer, int lenght, uint8_t flush_byte) {
	/* loop over required length */
	for (int i=0; i < lenght; i++) {
		/* Stimulate the next byte */
		while (!(SPI_USART->STATUS & USART_STATUS_TXBL))
			;
		SPI_USART->TXDATA = (uint32_t) flush_byte;
		/* wait and read one byte at a time */
		while (!(SPI_USART->STATUS & USART_STATUS_RXDATAV))
			;
		buffer[i] = (uint8_t) (SPI_USART->RXDATA);
	}
	/* return number of bytes read */
	return lenght;
}

int spi_write(uint8_t* buffer, int length) {
	for (int i=0; i < length; i++) {
		/* wait for last byte to send and the buffer new byte */
		while (!(SPI_USART->STATUS & USART_STATUS_TXBL))
			;
		SPI_USART->TXDATA = (uint32_t) buffer[i];
	}
	return length;
}

void spi_deinit(void) {
	/* Config the clocks ///////////////////////////////////////////////////// */
	// Disable USART clock
	CMU_ClockEnable(SPI_CLOCK, false);

	/* Config the USART ////////////////////////////////////////////////////// */
	// Disable Master, TX and RX
	SPI_USART->CMD = USART_CMD_MASTERDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;
	// Disable and clear interrupts
	SPI_USART->IEN = 0;
	NVIC_ClearPendingIRQ(SPI_IRQ_RXN);
	NVIC_ClearPendingIRQ(SPI_IRQ_TXN);
	NVIC_DisableIRQ(SPI_IRQ_RXN);
	NVIC_DisableIRQ(SPI_IRQ_TXN);
}

/* Inline function */
void spi_cs(bool enable) {
	if (enable) {
		GPIO->P[SPI_PORT].DOUTSET = 1 << SPI_PIN_CS;
	} else {
		GPIO->P[SPI_PORT].DOUTCLR = 1 << SPI_PIN_CS;
	}
}

void spi_clear_rx(void)
{
	/* TODO: Check if the while loop is needed and/or if pause is needed */
	while ((SPI_USART->STATUS & USART_STATUS_RXDATAV)) {
		SPI_USART->CMD = USART_CMD_CLEARRX;
	}
}

void spi_clear_tx(void)
{
	SPI_USART->CMD = USART_CMD_CLEARTX;
}

void spi_flush_tx(void)
{
	while (!(SPI_USART->STATUS & USART_STATUS_TXBL));
}

void SPI_IRQ_RX(void)
{
	if(SPI_USART->IF & USART_IF_RXOF) {
		// Recive overflow
		SPI_USART->IFC = USART_IFC_RXOF;
	}
}

void SPI_IRQ_TX(void)
{
	if(SPI_USART->IF & USART_IF_TXOF) {
		// Transmit overflow
		SPI_USART->IFC = USART_IFC_TXOF;
	}

	if(SPI_USART->IF & USART_IEN_TXC) {
		// Transmit complete: used to enable interrupt waits
		SPI_USART->IFC = USART_IEN_TXC;
	}
}

