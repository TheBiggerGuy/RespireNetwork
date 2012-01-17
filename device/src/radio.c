#include <stdint.h>

#include "efm32.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"
#include "efm32_usart.h"

#include "radio.h"

uint8_t Radio_read(uint8_t cmd);

#define RADIO_PORT gpioPortA
#define RADIO_TX  0
#define RADIO_RX  1
#define RADIO_CLK 2
#define RADIO_CS  3
#define RADIO_CE  4
#define RADIO_IRQ 6

void Radio_init(void)
{
	USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

	/* Wire up the GPOI ///////////////////////////////////////////////////// */

	// To avoid false start, configure output USART1_TX as high on PD0
	// Pin PD0 is configured to Push-pull
	GPIO_PinOutSet(RADIO_PORT, RADIO_TX);
	GPIO_PinModeSet(RADIO_PORT, RADIO_TX, gpioModeWiredAndPullUpFilter, true);

	// Pin PD1 is configured to Input enabled
	GPIO_PinModeSet(RADIO_PORT, RADIO_RX, gpioModeInput, false);

	// Pin PD2 is configured to Push-pull
	GPIO_PinModeSet(RADIO_PORT, RADIO_CLK, gpioModePushPull, true);

	// To avoid false start, configure output USART1_CS as high on PD3
	// Pin PD3 is configured to Push-pull
	GPIO_PinOutSet(RADIO_PORT, RADIO_CS);
	GPIO_PinModeSet(RADIO_PORT, RADIO_CS, gpioModePushPull, true);

	// Pin PD4 is configured to Push-pull
	GPIO_PinOutClear(RADIO_PORT, RADIO_CE);
	GPIO_PinModeSet(RADIO_PORT, RADIO_CE, gpioModePushPull, true);

	// Pin PD5 is configured to IRQ
	GPIO_PinModeSet(RADIO_PORT, RADIO_CE, gpioModeInput, false);

	// Module USART1 is configured to location 1
	USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK)
			| USART_ROUTE_LOCATION_LOC1;

	// Enable signals TX, RX, CLK, CS */
	USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN;

	/* init the clocks ///////////////////////////////////////////////////// */
	// Enable LE clock and LFRCO oscillator and Wait until LFRCO ready
	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

	// Select LFRCO as clock source for LFACLK
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Enable clock for USART1 */
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART1;

	/* Config UART (SPI) ///////////////////////////////////////////////////// */
	usartInit.msbf = true;
	usartInit.clockMode = usartClockMode3;
	usartInit.baudrate = 1000000; //7000000; // 7Mbits/s
	USART_InitSync(USART1, &usartInit);
	USART_Enable(USART1, usartEnable);

	/* Config Radio ///////////////////////////////////////////////////// */
	// TODO
	uint8_t bob[18] __attribute__((unused));
	for(int i=0; i<0x18; i++) {
		bob[i] = Radio_read(i);
	}

	/* Config IRQ //////////////////// */
	GPIO_IntConfig(RADIO_PORT, RADIO_IRQ, false, true, true);
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void USART_Rx_Clear(USART_TypeDef * usart) {
	// Make sure compiler keeps it in and stops it warning
	volatile uint8_t buf __attribute__((unused));

	while ((usart->STATUS & USART_STATUS_RXDATAV))
		{
			buf = usart->RXDATA;
		}
}

uint8_t Radio_read(uint8_t cmd)
{
	uint8_t buf;
	// CS low
	GPIO_PinOutClear(RADIO_PORT, RADIO_CS);
	// clear RX buffer
	USART_Rx_Clear(USART1);
	// send the cmd
	USART_Tx(USART1, cmd);
	// read one byte
	USART_Rx(USART1);
	// send byte to flush second byte
	USART_Tx(USART1, 0x00);
	// read the real data
	buf = USART_Rx(USART1);
	// CS high
	GPIO_PinOutSet(RADIO_PORT, RADIO_CS);

	return buf;
}

int Radio_Send(uint8_t* data)
{
	return -1;
}
