/**************************************************************************//**
 * @file
 * @brief LED driver code for Energy Micro EFM32_G8xx_STK starter kit
 * @author Energy Micro AS
 * @version 2.1.3
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
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

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

#include "efm32.h"
#include "leds.h"
#include "efm32_gpio.h"
#include "efm32_cmu.h"

#define LEDPORT gpioPortC

/**************************************************************************//**
 * @brief Light up LED
 * @param led LED number (0-3)
 *****************************************************************************/
void LED_Set(int led)
{
  if ((led >= 0) && (led < 4))
  {
    GPIO_PinOutSet(LEDPORT, led);
  }
}

/**************************************************************************//**
 * @brief Turn off LED
 * @param led LED number (0-3)
 *****************************************************************************/
void LED_Clear(int led)
{
  if ((led >= 0) && (led < 4))
  {
    GPIO_PinOutClear(LEDPORT, led);
  }
}

/**************************************************************************//**
 * @brief Toggle LED, switch from on to off or vice versa
 * @param led LED number (0-3)
 *****************************************************************************/
void LED_Toggle(int led)
{
  if ((led >= 0) && (led < 4))
  {
    GPIO_PinOutToggle(LEDPORT, led);
  }
}


/**************************************************************************//**
 * @brief Light up LEDs according value of 4 least significat bits
 * @param value Bit pattern
 *****************************************************************************/
void LED_Value(int value)
{
  /* Set the value directly using 0xf as a mask. */
  GPIO_PortOutSetVal(LEDPORT, value, 0xf);
}


/**************************************************************************//**
 * @brief Initialize LED interface
 *****************************************************************************/
void LED_Init(void)
{
  /* Enable GPIO */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure GPIO port C 0-3 as LED control outputs */
  /* Enable the LED by default */
  GPIO_PinModeSet(LEDPORT, 0, gpioModePushPull, 1);
  GPIO_PinModeSet(LEDPORT, 1, gpioModePushPull, 1);
  GPIO_PinModeSet(LEDPORT, 2, gpioModePushPull, 1);
  GPIO_PinModeSet(LEDPORT, 3, gpioModePushPull, 1);
}

/** @} (end group Drivers) */
