/**************************************************************************//**
 * @file
 * @brief Segment LCD Font And Layout for the EFM32_Gxxx_STK starter kit
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
 ****************************************************************************/
#include "segmentfont.h"

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @brief Working instance of LCD display
 *****************************************************************************/
const MCU_DISPLAY EFM_Display = {
  .Text        = {
    { /* 1 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 10, .bit[1] = 12, .bit[2] = 12, .bit[3] = 10,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 9, .bit[5] = 9, .bit[6] = 9, .bit[7] = 10,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 11, .bit[9] = 11, .bit[10] = 12, .bit[11] = 11,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 11, .bit[13] = 10
    },
    { /* 2 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 14, .bit[1] = 16, .bit[2] = 16, .bit[3] = 14,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 13, .bit[5] = 13, .bit[6] = 13, .bit[7] = 14,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 15, .bit[9] = 15, .bit[10] = 16, .bit[11] = 15,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 15, .bit[13] = 14
    },
    { /* 3 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 18, .bit[1] = 20, .bit[2] = 20, .bit[3] = 18,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 17, .bit[5] = 17, .bit[6] = 17, .bit[7] = 18,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 19, .bit[9] = 19, .bit[10] = 20, .bit[11] = 19,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 19, .bit[13] = 18
    },
    { /* 4 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 22, .bit[1] = 24, .bit[2] = 24, .bit[3] = 22,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 21, .bit[5] = 21, .bit[6] = 21, .bit[7] = 22,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 23, .bit[9] = 23, .bit[10] = 24, .bit[11] = 23,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 23, .bit[13] = 22
    },
    { /* 5 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 25, .bit[1] = 6, .bit[2] = 6, .bit[3] = 25,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 7, .bit[5] = 7, .bit[6] = 7, .bit[7] = 25,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 26, .bit[9] = 26, .bit[10] = 6, .bit[11] = 26,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 26, .bit[13] = 25
    },
    { /* 6 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 27, .bit[1] = 04, .bit[2] = 04, .bit[3] = 27,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 5, .bit[5] = 5, .bit[6] = 5, .bit[7] = 27,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 28, .bit[9] = 28, .bit[10] = 4, .bit[11] = 28,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 28, .bit[13] = 27
    },
    { /* 7 */
      .com[0] = 3, .com[1] = 3, .com[2] = 1, .com[3] = 0,
      .bit[0] = 29, .bit[1] = 2, .bit[2] = 2, .bit[3] = 29,

      .com[4] = 1, .com[5] = 3, .com[6] = 2, .com[7] = 2,
      .bit[4] = 03, .bit[5] = 3, .bit[6] = 3, .bit[7] = 29,

      .com[8] = 2, .com[9] = 3, .com[10] = 2, .com[11] = 0,
      .bit[8] = 30, .bit[9] = 30, .bit[10] = 2, .bit[11] = 30,

      .com[12] = 1, .com[13] = 1,
      .bit[12] = 30, .bit[13] = 29
    }
  },
  .Number      = {
    {
      .com[0] = 3, .com[1] = 2, .com[2] = 1, .com[3] = 0,
      .bit[0] = 31, .bit[1] = 31, .bit[2] = 31, .bit[3] = 31,

      .com[4] = 1, .com[5] = 3, .com[6] = 2,
      .bit[4] = 32, .bit[5] = 32, .bit[6] = 32,
    },
    {
      .com[0] = 3, .com[1] = 2, .com[2] = 1, .com[3] = 0,
      .bit[0] = 33, .bit[1] = 33, .bit[2] = 33, .bit[3] = 33,

      .com[4] = 1, .com[5] = 3, .com[6] = 2,
      .bit[4] = 35, .bit[5] = 35, .bit[6] = 35,
    },
    {
      .com[0] = 3, .com[1] = 2, .com[2] = 1, .com[3] = 0,
      .bit[0] = 36, .bit[1] = 36, .bit[2] = 36, .bit[3] = 36,

      .com[4] = 1, .com[5] = 3, .com[6] = 2,
      .bit[4] = 37, .bit[5] = 37, .bit[6] = 37,
    },
    {
      .com[0] = 3, .com[1] = 2, .com[2] = 1, .com[3] = 0,
      .bit[0] = 38, .bit[1] = 38, .bit[2] = 38, .bit[3] = 38,

      .com[4] = 1, .com[5] = 3, .com[6] = 2,
      .bit[4] = 39, .bit[5] = 39, .bit[6] = 39,
    },
  },
  .EMode       = {
    .com[0] = 1, .bit[0] = 1,
    .com[1] = 2, .bit[1] = 1,
    .com[2] = 1, .bit[2] = 0,
    .com[3] = 2, .bit[3] = 0,
    .com[4] = 3, .bit[4] = 0,
  },
  .ARing       = {
    .com[0] = 0, .bit[0] = 0,
    .com[1] = 0, .bit[1] = 1,
    .com[2] = 0, .bit[2] = 2,
    .com[3] = 0, .bit[3] = 3,

    .com[4] = 0, .bit[4] = 4,
    .com[5] = 0, .bit[5] = 5,
    .com[6] = 0, .bit[6] = 6,
    .com[7] = 0, .bit[7] = 7,
  },
  .Battery     = {
    .com[0] = 0, .bit[0] = 12,
    .com[1] = 0, .bit[1] = 17,
    .com[2] = 0, .bit[2] = 20,
    .com[3] = 0, .bit[3] = 13,
  }
};

/**************************************************************************//**
 * @brief
 * Defines higlighted segments for the alphabet, starting from "blank" (SPACE)
 * Uses bit pattern as defined for text segments above.
 * E.g. a capital O, would have bits 0 1 2 3 4 5 => 0x003f defined
 *****************************************************************************/
const uint16_t EFM_Alphabet[] = {
  0x0000, /* space */
  0x1100, /* ! */
  0x0280, /* " */
  0x0000, /* # */
  0x0000, /* $ */
  0x0602, /* % */
  0x0000, /* & */
  0x0020, /* ' */
  0x0039, /* ( */
  0x000f, /* ) */
  0x0000, /* * */
  0x1540, /* + */
  0x2000, /* , */
  0x0440, /* - */
  0x1000, /* . */
  0x2200, /* / */

  0x003f, /* 0 */
  0x0006, /* 1 */
  0x045b, /* 2 */
  0x044f, /* 3 */
  0x0466, /* 4 */
  0x046d, /* 5 */
  0x047d, /* 6 */
  0x0007, /* 7 */
  0x047f, /* 8 */
  0x046f, /* 9 */

  0x0000, /* : */
  0x0000, /* ; */
  0x0a00, /* < */
  0x0000, /* = */
  0x2080, /* > */
  0x0000, /* ? */
  0xffff, /* @ */

  0x0477, /* A */
  0x0a79, /* B */
  0x0039, /* C */
  0x20b0, /* D */
  0x0079, /* E */
  0x0071, /* F */
  0x047d, /* G */
  0x0476, /* H */
  0x0006, /* I */
  0x000e, /* J */
  0x0a70, /* K */
  0x0038, /* L */
  0x02b6, /* M */
  0x08b6, /* N */
  0x003f, /* O */
  0x0473, /* P */
  0x083f, /* Q */
  0x0c73, /* R */
  0x046d, /* S */
  0x1101, /* T */
  0x003e, /* U */
  0x2230, /* V */
  0x2836, /* W */
  0x2a80, /* X */
  0x046e, /* Y */
  0x2209, /* Z */

  0x0039, /* [ */
  0x0880, /* backslash */
  0x000f, /* ] */
  0x0001, /* ^ */
  0x0008, /* _ */
  0x0100, /* ` */

  0x1058, /* a */
  0x047c, /* b */
  0x0058, /* c */
  0x045e, /* d */
  0x2058, /* e */
  0x0471, /* f */
  0x0c0c, /* g */
  0x0474, /* h */
  0x0004, /* i */
  0x000e, /* j */
  0x0c70, /* k */
  0x0038, /* l */
  0x1454, /* m */
  0x0454, /* n */
  0x045c, /* o */
  0x0473, /* p */
  0x0467, /* q */
  0x0450, /* r */
  0x0c08, /* s */
  0x0078, /* t */
  0x001c, /* u */
  0x2010, /* v */
  0x2814, /* w */
  0x2a80, /* x */
  0x080c, /* y */
  0x2048, /* z */

  0x0000,
};

/**************************************************************************//**
 * @brief
 * Defines higlighted segments for the numeric display
 *****************************************************************************/
const uint16_t EFM_Numbers[] =
{
  0x003f, /* 0 */
  0x0006, /* 1 */
  0x005b, /* 2 */
  0x004f, /* 3 */
  0x0066, /* 4 */
  0x006d, /* 5 */
  0x007d, /* 6 */
  0x0007, /* 7 */
  0x007f, /* 8 */
  0x006f, /* 9 */
  0x0040, /* - */
};

/** @} (end group Drivers) */
