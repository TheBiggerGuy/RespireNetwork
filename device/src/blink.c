/**************************************************************************//**
 * @file
 * @brief Simple LCD blink demo for EFM32_Gxxx_STK
 * @author Energy Micro AS
 * @version 2.1.3
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/

// STK
#ifndef EFM32G890F128
#define EFM32G890F128
#endif
// Respire
//#ifndef EFM32G210F128
//define EFM32G210F128
//#endif

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "efm32.h"
#include "efm32_chip.h"
#include "efm32_dbg.h"

#include "dbg.h"

#ifdef EFM32G890F128
/* LED driver */
#include "leds.h"
#else
void
LED_Init(void)
  {
  }
;
void
LED_Set(int led)
  {
  }
;
void
LED_Clear(int led)
  {
  }
;
void
LED_Toggle(int led)
  {
  }
;
void
LED_Value(int value)
  {
  }
;
#endif

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void
SysTick_Handler(void)
{
  msTicks++; /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void
Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks)
    ;
}

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
    while (1)
      ;

  setupSWO();

  /* Initialize LED driver */
  LED_Init();

  loopme:

  //ITM_SendChar('A');
  //ITM_SendChar2('A');

  printf("hello\n");

  goto loopme;

  NVIC_SystemReset();
}
