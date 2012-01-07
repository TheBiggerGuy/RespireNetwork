/**************************************************************************//**
 * @file
 * @brief STK Segment LCD Display driver
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "efm32.h"
#include "efm32_cmu.h"
#include "efm32_lcd.h"
#include "segmentlcd.h"
#include "segmentfont.h"

/**************************************************************************//**
 * @brief Write number on numeric part on Segment LCD display
 * @param value Numeric value to put on display, in range -999 to +9999
 *****************************************************************************/
void SegmentLCD_Number(int value)
{
  int      num, i, com, bit, digit, div, neg;
  uint16_t bitpattern;

  /* Parameter consistancy check */
  if (value >= 9999)
  {
    value = 9999;
  }
  if (value <= -1000)
  {
    value = -999;
  }
  if (value < 0)
  {
    value = abs(value);
    neg   = 1;
  }
  else
  {
    neg = 0;
  }

  /* If an update is in progress we must block, or there might be tearing */
  LCD_SyncBusyDelay(0xFFFFFFFF);

  /* Freeze updates to avoid partial refresh of display */
  LCD_FreezeEnable(true);

  /* Turn off all number LCD segments */
  SegmentLCD_NumberOff();

  /* Extract useful digits */
  div = 1;
  for (digit = 0; digit < 4; digit++)
  {
    num = (value / div) % 10;
    if ((neg == 1) && (digit == 3)) num = 10;
    /* Get number layout of display */
    bitpattern = EFM_Numbers[num];
    for (i=0; i<7; i++)
    {
      bit = EFM_Display.Number[digit].bit[i];
      com = EFM_Display.Number[digit].com[i];
      if (bitpattern & (1 << i))
      {
        LCD_SegmentSet(com, bit, true);
      }
    }
    div = div * 10;
  }
  /* Sync LCD registers to LE domain */
  LCD_FreezeEnable(false);
}

/**************************************************************************//**
 * @brief Turn all segments on numeric digits in display off
 *****************************************************************************/
void SegmentLCD_NumberOff(void)
{
  /* Turn off all number segments */
  LCD_SegmentSetLow(0, 0x80000000, 0x00000000);
  LCD_SegmentSetLow(1, 0x80000000, 0x00000000);
  LCD_SegmentSetLow(2, 0x80000000, 0x00000000);
  LCD_SegmentSetLow(3, 0x80000000, 0x00000000);
  LCD_SegmentSetHigh(0, 0x52, 0x00000000);
  LCD_SegmentSetHigh(1, 0xFB, 0x00000000);
  LCD_SegmentSetHigh(2, 0xFB, 0x00000000);
  LCD_SegmentSetHigh(3, 0xFB, 0x00000000);
}

/**************************************************************************//**
 * @brief Turn all segments on alpha characters in display off
 *****************************************************************************/
void SegmentLCD_AlphaNumberOff(void)
{
  /* Turn off all number segments */
  LCD_SegmentSetLow(0, 0x7ECCCC00, 0x00000000);
  LCD_SegmentSetLow(1, 0x7FFFFEFC, 0x00000000);
  LCD_SegmentSetLow(2, 0x7FFFFEFC, 0x00000000);
  LCD_SegmentSetLow(3, 0x7FFFFEFC, 0x00000000);
  return;
}

/**************************************************************************//**
 * @brief Write text on LCD display
 * @param string Text string to show on display
 *****************************************************************************/
void SegmentLCD_Write(char *string)
{
  int      data, length, index;
  uint16_t bitfield;
  uint32_t com, bit;
  int      i;

  length = strlen(string);
  index  = 0;

  /* If an update is in progress we must block, or there might be tearing */
  LCD_SyncBusyDelay(0xFFFFFFFF);
  
  /* Freeze LCD to avoid partial updates */
  LCD_FreezeEnable(true);

  /* Turn all segments off */
  SegmentLCD_AlphaNumberOff();

  /* fill out all characters on display */
  for (index=0; index<7; index++)
  {
    if (index<length)
    {
      data = (int) *string;
    }
    else           /* padding with space */
    {
      data = 0x20; /* SPACE */
    }
    /* defined letters currently starts at "SPACE" - ASCII 0x20; */
    data     = data - 0x20;
    /* get font for this letter */
    bitfield = EFM_Alphabet[data];

    for (i = 0; i < 14; i++)
    {
      bit   = EFM_Display.Text[index].bit[i];
      com   = EFM_Display.Text[index].com[i];

      if (bitfield & (1 << i))
      {
        /* Turn on segment */
        LCD_SegmentSet(com, bit, true);
      }
    }
    string++;
  }
  /* Enable update */
  LCD_FreezeEnable(false);
}

/**************************************************************************//**
 * @brief Disable all segments
 *****************************************************************************/
void SegmentLCD_AllOff(void)
{
  LCD_SegmentSetLow(0, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(1, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(2, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(3, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetHigh(0, 0xFF, 0x00000000);
  LCD_SegmentSetHigh(1, 0xFF, 0x00000000);
  LCD_SegmentSetHigh(2, 0xFF, 0x00000000);
  LCD_SegmentSetHigh(3, 0xFF, 0x00000000);
}

/**************************************************************************//**
 * @brief Enable all segments
 *****************************************************************************/
void SegmentLCD_AllOn(void)
{
  LCD_SegmentSetLow(0, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(1, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(2, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(3, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetHigh(0, 0xFF, 0xFFFFFFFF);
  LCD_SegmentSetHigh(1, 0xFF, 0xFFFFFFFF);
  LCD_SegmentSetHigh(2, 0xFF, 0xFFFFFFFF);
  LCD_SegmentSetHigh(3, 0xFF, 0xFFFFFFFF);
}

/**************************************************************************//**
 * @brief Light up or shut off Energy Mode indicator
 * @param em Energy Mode numer 0 to 4
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_EnergyMode(int em, int on)
{
  uint32_t com, bit;

  com      = EFM_Display.EMode.com[em];
  bit = EFM_Display.EMode.bit[em];

  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}

/**************************************************************************//**
 * @brief Light up or shut off Ring of Indicators
 * @param anum "Segment number" on "Ring", range 0 - 7
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_ARing(int anum, int on)
{
  uint32_t com, bit;

  com      = EFM_Display.ARing.com[anum];
  bit = EFM_Display.ARing.bit[anum];

  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}

/**************************************************************************//**
 * @brief Light up or shut off various symbols on Segment LCD
 * @param s Which symbol to turn on or off
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_Symbol(lcdSymbol s, int on)
{
  int com = 0;
  int bit = 0;

  switch (s)
  {
  case LCD_SYMBOL_GECKO:
    com = 3; bit = 8;
    break;
  case LCD_SYMBOL_ANT:
    com = 3; bit = 1;
    break;
  case LCD_SYMBOL_PAD0:
    com = 1; bit = 8;
    break;
  case LCD_SYMBOL_PAD1:
    com = 2; bit = 8;
    break;
  case LCD_SYMBOL_AM:
    com = 0; bit = 32;
    break;
  case LCD_SYMBOL_PM:
    com = 0; bit = 35;
    break;
  case LCD_SYMBOL_EFM32:
    com = 0; bit = 8;
    break;
  case LCD_SYMBOL_MINUS:
    com = 0; bit = 9;
    break;
  case LCD_SYMBOL_COL3:
    com = 0; bit = 16;
    break;
  case LCD_SYMBOL_COL5:
    com = 0; bit = 24;
    break;
  case LCD_SYMBOL_COL10:
    com = 0; bit = 39;
    break;
  case LCD_SYMBOL_DEGC:
    com = 0; bit = 34;
    break;
  case LCD_SYMBOL_DEGF:
    com = 1; bit = 34;
    break;
  case LCD_SYMBOL_MICROAMP:
    com = 2; bit = 34;
    break;
  case LCD_SYMBOL_MILLIAMP:
    com = 3; bit = 34;
    break;
  case LCD_SYMBOL_DP6:
    com = 0; bit = 21;
    break;
  case LCD_SYMBOL_DP10:
    com = 0; bit = 37;
    break;
  }
  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}

/**************************************************************************//**
 * @brief Light up or shut off Battery Indicator
 * @param batteryLevel Battery Level, 0 to 4 (0 turns all off)
 *****************************************************************************/
void SegmentLCD_Battery(int batteryLevel)
{
  uint32_t com, bit;
  int      i, on;

  for (i = 0; i < 4; i++)
  {
    if (i < batteryLevel)
    {
      on = 1;
    }
    else
    {
      on = 0;
    }
    com = EFM_Display.Battery.com[i];
    bit = EFM_Display.Battery.bit[i];

    if (on)
    {
      LCD_SegmentSet(com, bit, true);
    }
    else
    {
      LCD_SegmentSet(com, bit, false);
    }
  }
}

/**************************************************************************//**
 * @brief Segment LCD Initialization routine for EFM32 STK display
 * @param useBoost Set to use voltage boost
 *****************************************************************************/
void SegmentLCD_Init(bool useBoost)
{
  LCD_Init_TypeDef lcd = LCD_INIT_DEFAULT;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Enable LFXO as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  /* Enable clock to LCD module */
  CMU_ClockEnable(cmuClock_LCD, true);
  /* LCD Controller Prescaler (divide by 128) */
  /* CLKlcd = 256 Hz */
  CMU_ClockDivSet(cmuClock_LCDpre, cmuClkDiv_128);

  /* Disable interrupts */
  LCD_IntDisable(0xFFFFFFFF);
  /* Initialize and enable LCD controller */
  LCD_Init(&lcd);
  /* Enable all display segments */
  LCD_SegmentRangeEnable(lcdSegmentAll, true);
  /* Enable boost if necessary */
  if ( useBoost ) {
    LCD_VBoostSet(lcdVBoostLevel0);
    LCD_VLCDSelect(lcdVLCDSelVExtBoost);
    CMU->LCDCTRL |= CMU_LCDCTRL_VBOOSTEN;
  }
  /* Turn all segments off */
  SegmentLCD_AllOff();
  
  LCD_SyncBusyDelay(0xFFFFFFFF);
}


/**************************************************************************//**
 * @brief Disables LCD controller
 *****************************************************************************/
void SegmentLCD_Disable(void)
{
  /* Disable LCD */
  LCD_Enable(false);

  /* Make sure CTRL register has been updated */
  LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);

  /* Turn off LCD clock */
  CMU_ClockEnable(cmuClock_LCD, false);

  /* Turn off voltage boost if enabled */
  CMU->LCDCTRL = 0;
}

/** @} (end group Drivers) */

