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

#include "dbg.h"
#include "radio.h"

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int
main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClock / 1000))
    exit(-1);

  DBG_Init();

  Radio_init();

  while (1)
    {
	  LOG_DEBUG("loop\n");
      Radio_Send("bob");
    }

  exit(0);
}
