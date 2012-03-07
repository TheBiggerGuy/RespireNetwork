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

#define IS_BASE

#include "main.h"
#include "dbg.h"
#if defined(IS_BASE)
#include "net_base.h"
#else
#include "net_node.h"
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

#if defined(IS_BASE)
void (*init_list[])(void)   = {DBG_init, net_base_init, RTC_init, NULL};
void (*deinit_list[])(void) = {RTC_deinit, net_base_deinit, DBG_deinit, NULL};
#else
void (*init_list[])(void)   = {DBG_init, net_node_init, RTC_init, NULL};
void (*deinit_list[])(void) = {RTC_deinit, net_node_deinit, DBG_deinit, NULL};
#endif

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

	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(SystemCoreClock / 1400))
		exit(EXIT_FAILURE);

	/* Run the module initilizers */
	i = 0;
	//while(init_list[i] != NULL) {
	//	init_list[i]();
	//	i++;
	//}
	DBG_init();
	RTC_init();

	// printf("Finished Init\n");

	// TODO
	RTC_setTime(1328288470);

	while (DO_MAIN_LOOP == true) {
		// DBG_LED_Toggle();
	}

	printf("Starting DeInit\n");

	/* Run the module de-initilizers */
	i = 0;
	while(deinit_list[i] != NULL) {
		deinit_list[i]();
		i++;
	}

	exit(EXIT_SUCCESS);
}
