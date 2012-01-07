/**************************************************************************//**
 * @file
 * @brief Capacitive sense driver
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
#ifndef __CAPSENSE_H_
#define __CAPSENSE_H_

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define ACMP_CHANNELS           8

#define BUTTON_CHANNEL          5
#define SLIDER_PART0_CHANNEL    0
#define SLIDER_PART1_CHANNEL    1
#define SLIDER_PART2_CHANNEL    2
#define SLIDER_PART3_CHANNEL    3

uint32_t CAPSENSE_getVal(uint8_t channel);
uint32_t CAPSENSE_getNormalizedVal(uint8_t channel);
bool CAPSENSE_getPressed(uint8_t channel);
int32_t CAPSENSE_getSliderPosition(void);
void CAPSENSE_Sense(void);
void CAPSENSE_Init(void);

#ifdef __cplusplus
}
#endif

/** @} (end group Drivers) */

#endif /* __CAPSENSE_H_ */


