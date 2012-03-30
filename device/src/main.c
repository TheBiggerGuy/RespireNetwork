/**************************************************************************//**
 * @file
 * @brief The Main entry point into the program.
 * @author Guy Taylor
 * @version 0.2.1
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

#include "config.h"
#include "main.h"
#include "dbg.h"

#include "net_packets.h"
#if CONFIG_FUNC == CONFIG_FUNC_BASE
#include "net_base.h"
#elif CONFIG_FUNC == CONFIG_FUNC_NODE
#include "net_node.h"
#elif CONFIG_FUNC == CONFIG_FUNC_TEST
#include "net_test.h"
#else
#error "Invalid 'CONFIG_FUNC'"
#endif
#include "rtc.h"

/*
 * E10 RX
 * E11 TX
 * E12 CLK
 * E13 CSN
 * 
 * E14 IRQ
 * E15 CE
 */

#if CONFIG_FUNC == CONFIG_FUNC_BASE
void (*init_list[])(void)   = {DBG_init, RTC_init, net_base_init, NULL};
void (*deinit_list[])(void) = {RTC_deinit, net_base_deinit, DBG_deinit, NULL};
#elif CONFIG_FUNC == CONFIG_FUNC_NODE
void (*init_list[])(void)   = {DBG_init, RTC_init, net_node_init, NULL};
void (*deinit_list[])(void) = {RTC_deinit, net_node_deinit, DBG_deinit, NULL};
#elif CONFIG_FUNC == CONFIG_FUNC_TEST
void (*init_list[])(void)   = {DBG_init, RTC_init, NULL};
void (*deinit_list[])(void) = {RTC_deinit, DBG_deinit, NULL};
#else
#error "Invalid 'CONFIG_FUNC'"
#endif

volatile uint8_t msTicks; /* counts 1ms timeTicks */

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
	uint8_t till;
	/* Setup SysTick Timer for 1 msec interrupts  */
	SysTick_Config(SystemCoreClock / 1400);
	till = (msTicks + dlyTicks);
	while (msTicks != till) {
		__WFI();
	}
	SysTick->CTRL = 0x00;
	NVIC_DisableIRQ(SysTick_IRQn);
}

volatile bool DO_MAIN_LOOP = true;

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void) {
	int i;

	/* Chip errata */
	CHIP_Init();

	/* Ensure core frequency has been updated */
	SystemCoreClockUpdate();

	/* Run the module initilizers */
	i = 0;
	while(init_list[i] != NULL) {
		init_list[i]();
		i++;
	}

	printf("Finished Init\n");

	// TODO
	RTC_setTime(1328288470);

#if CONFIG_FUNC == CONFIG_FUNC_BASE
	net_base_run();
#elif CONFIG_FUNC == CONFIG_FUNC_NODE
	net_node_join();
	net_node_run();
#elif CONFIG_FUNC == CONFIG_FUNC_TEST
	net_test_init(NET_TEST_RX_ONLY);
#endif

	printf("Starting DeInit\n");

	/* Run the module de-initilizers */
	i = 0;
	while(deinit_list[i] != NULL) {
		deinit_list[i]();
		i++;
	}

	exit(EXIT_SUCCESS);
}
