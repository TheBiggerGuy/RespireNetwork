/**************************************************************************//**
 * @file
 * @brief Magic
 * @author Guy Taylor
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2011-2012 Guy Taylor
 *****************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "efm32.h"
#include "system_efm32.h"
#include "efm32_chip.h"
#include "efm32_dbg.h"

#include "main.h"
#include "dbg.h"
#include "radio.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void delay(uint32_t dlyTicks)
{
  uint32_t till = msTicks + dlyTicks;
  while (msTicks < till) {
  	//__WFI();
  	__NOP();
  }
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int
main(void)
{
	uint8_t data[16];

  /* Chip errata */
  CHIP_Init();

  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1400))
    exit(-1);

  DBG_Init();

  Radio_init();

  while (1) {
	  for(int i=0; i< 5; i++) {
		  Radio_send((uint8_t*) "bob\x01\x02", 0, 5);
		  if(Radio_available() > 0) {
			  Radio_recive(data, 16);
			  LOG_DEBUG("data: %s\n", data);
		  }
	  }
	  delay(5*1000);
  }
  exit(0);
}
