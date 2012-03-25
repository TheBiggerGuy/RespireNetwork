/**************************************************************************//**
 * @file
 * @brief The Main entry point into the program.
 * @author Guy Taylor
 * @version 0.1.1
 ******************************************************************************
 * @section License
 * Copyright 2011 Guy Taylor <guy@thebiggerguy.com>
 * 
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * NOTE:
 *   * Pre  7th March 2012: Copyright Guy Taylor
 *   * Post 7th March 2012: Copyright Guy Taylor with "Apache License, Ver 2.0"
 *****************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "efm32.h"
#include "system_efm32.h"
#include "efm32_chip.h"
#include "efm32_dbg.h"

#include "radio.h"
#include "config.h"
#include "main.h"
#include "spi.h"

volatile time_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 * This keeps a record of the number of ms the system has been active
 *****************************************************************************/
void SysTick_Handler(void) {
	++msTicks; /* increment counter necessary in delay() */
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void delay(uint8_t dlyTicks) {
	time_t till = msTicks + dlyTicks;
	while (msTicks < till) {
		//__WFI();
		__NOP();
	}
}

void radio_ce(bool state)
{
	if (state) {
		GPIO->P[RADIO_PORT_CE].DOUTSET = 1 << RADIO_PIN_CE;
	} else {
		GPIO->P[RADIO_PORT_CE].DOUTCLR = 1 << RADIO_PIN_CE;
	}
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void) {

	/* Chip errata */
	CHIP_Init();

	/* Ensure core frequency has been updated */
	SystemCoreClockUpdate();

	/* Setup SysTick Timer for 1 msec interrupts
	 * Note:
	 *   This enables the IRQ in the IRQ table
     */
	if (SysTick_Config(SystemCoreClock / 1400))
		exit(EXIT_FAILURE);

	GPIO_PinModeSet(RADIO_PORT_CE,  RADIO_PIN_CE,  gpioModeInputPull, 0);  // CE
	GPIO_PinModeSet(RADIO_PORT_IRQ, RADIO_PIN_IRQ, gpioModeInput,     0);  // IRQ

	radio_ce(false);

	spi_init();

	spi_clear_rx();
	spi_clear_tx();

	while(true) {
		//net_send();
		delay(100);
		net_read_fifo();
	}

	exit(EXIT_SUCCESS);
}
