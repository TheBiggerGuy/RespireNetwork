/**************************************************************************//**
 * @file
 * @brief Segment LCD Display driver for STK, prototypes and definitions
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
#ifndef __SEGMENTLCD_H
#define __SEGMENTLCD_H

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/** Range of symbols available on display */
typedef enum
{
  LCD_SYMBOL_GECKO,
  LCD_SYMBOL_ANT,
  LCD_SYMBOL_PAD0,
  LCD_SYMBOL_PAD1,
  LCD_SYMBOL_AM,
  LCD_SYMBOL_PM,
  LCD_SYMBOL_EFM32,
  LCD_SYMBOL_MINUS,
  LCD_SYMBOL_COL3,
  LCD_SYMBOL_COL5,
  LCD_SYMBOL_COL10,
  LCD_SYMBOL_DEGC,
  LCD_SYMBOL_DEGF,
  LCD_SYMBOL_MICROAMP,
  LCD_SYMBOL_MILLIAMP,
  LCD_SYMBOL_DP6,
  LCD_SYMBOL_DP10,
} lcdSymbol;

#ifdef __cplusplus
extern "C" {
#endif

/* LCD Interrupt handler */
void LCD_IRQHandler(void);

/* Regular functions */
void SegmentLCD_Init(bool useBoost);
void SegmentLCD_Disable(void);
void SegmentLCD_AllOff(void);
void SegmentLCD_AllOn(void);
void SegmentLCD_ARing(int anum, int on);
void SegmentLCD_Battery(int batteryLevel);
void SegmentLCD_EnergyMode(int em, int on);
void SegmentLCD_Number(int value);
void SegmentLCD_NumberOff(void);
void SegmentLCD_AlphaNumberOff(void);
void SegmentLCD_Symbol(lcdSymbol s, int on);
void SegmentLCD_Write(char *string);

#ifdef __cplusplus
}
#endif

/** @} (end group Drivers) */

#endif

