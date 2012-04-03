/***************************************************************************//**
 * @file
 * @brief Backup Real Time Counter (BURTC) Peripheral API for EFM32
 * @author Energy Micro AS
 * @version 2.4.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
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
 ******************************************************************************/
#include "efm32.h"
#if defined(BURTC_PRESENT)
#include "efm32_burtc.h"

#include "efm32_assert.h"
#include "efm32_bitband.h"

/***************************************************************************//**
 * @addtogroup EFM32_Library
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup BURTC
 * @brief Backup Real Time Counter (BURTC) Peripheral API for EFM32
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief Convert dividend to prescaler logarithmic value. Only works for even
 *        numbers equal to 2^n
 * @param[in] div Unscaled dividend,
 * @return Base 2 logarithm of input, as used by fixed prescalers
 ******************************************************************************/
static __INLINE uint32_t BURTC_DivToLog2(uint32_t div)
{
  uint32_t log2;

  /* Prescaler accepts an argument of 128 or less, valid values being 2^n */
  EFM_ASSERT((div > 0) && (div <= 32768));

  /* Count leading zeroes and "reverse" result, Cortex-M3 intrinsic */
  log2 = (31 - __CLZ(div));

  return log2;
}


/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief Initialize BURTC
 *
 * @details
 *    Configures the BURTC peripheral.
 *
 * @note
 *   Before initialization, BURTC module must first be enabled by clearing the 
 *   reset bit in the RMU, i.e.
 * @verbatim
 *   RMU_ResetControl(rmuResetBU, false);
 * @endverbatim
 *   Compare channel 0 must be configured outside this function, before
 *   initialization if enable is set to true. The counter will always be reset.
 *
 * @param[in] burtcInit
 *   Pointer to BURTC initialization structure
 ******************************************************************************/
void BURTC_Init(const BURTC_Init_TypeDef *burtcInit)
{
  uint32_t ctrl;
  uint32_t presc;

  /* Check initializer structure integrity */
  EFM_ASSERT(burtcInit != (BURTC_Init_TypeDef *) 0);
  /* Clock divider must be between 1 and 128, really on the form 2^n */
  EFM_ASSERT((burtcInit->clkDiv >= 1) && (burtcInit->clkDiv <= 128));
  /* Ignored compare bits during low power operation must be less than 7 */
  /* Note! Giant Gecko revision C errata, do NOT use LPCOMP=7 */
  EFM_ASSERT(burtcInit->lowPowerComp <= 6);
  /* You cannot enable the BURTC if mode is set to disabled */
  EFM_ASSERT((burtcInit->enable == false) ||
             ((burtcInit->enable == true) && (burtcInit->mode != burtcModeDisable)));
  /* Low power mode is only available with LFRCO or LFXO as clock source */
  EFM_ASSERT((burtcInit->clkSel != burtcClkSelULFRCO) ||
             ((burtcInit->clkSel == burtcClkSelULFRCO) && (burtcInit->lowPowerMode == burtcLPDisable)));

  /* Calculate prescaler value from clock divider input */
  /* Note! If clock select (clkSel) is ULFRCO, a non-zero clkDiv will select */
  /* 1kHz clock source, while any other value will use a 2kHz ULFRCO clock */
  presc = BURTC_DivToLog2(burtcInit->clkDiv);

  /* Make sure all registers are updated simultaneously */
  BURTC_FreezeEnable(true);

  /* Configure low power mode */
  BURTC->LPMODE = (uint32_t)(burtcInit->lowPowerMode);

  /* New configuration */
  ctrl = ((BURTC_CTRL_RSTEN) | 
          (burtcInit->mode) |
          (burtcInit->debugRun << _BURTC_CTRL_DEBUGRUN_SHIFT) |
          (burtcInit->compare0Top << _BURTC_CTRL_COMP0TOP_SHIFT) |
          (burtcInit->lowPowerComp << _BURTC_CTRL_LPCOMP_SHIFT) |
          (presc << _BURTC_CTRL_PRESC_SHIFT) |
          (burtcInit->clkSel) |
          (burtcInit->timeStamp << _BURTC_CTRL_BUMODETSEN_SHIFT));

  /* Clear interrupts */
  BURTC->IFC = 0xFFFFFFFF;

  /* Set new configuration */
  BURTC->CTRL = ctrl;

  /* Enable BURTC and counter */
  if (burtcInit->enable)
  {
    /* To enable BURTC counter, we need to disable reset */
    BURTC_Enable(true);
  }

  /* Clear freeze */
  BURTC_FreezeEnable(false);

  /* Await synchronization into low frequency domain */
  while (BURTC->SYNCBUSY) ;
}


/***************************************************************************//**
 * @brief Set BURTC compare channel
 *
 * @param[in] comp Compare channel index, must be 0 for Giant / Leopard Gecko
 *
 * @param[in] value New compare value
 ******************************************************************************/
void BURTC_CompareSet(unsigned int comp, uint32_t value)
{
  EFM_ASSERT(comp == 0);

  /* Configure compare channel 0 */
  BURTC->COMP0 = value;

  /* Check if freeze or RSTEN is active, if not wait for synchronization of register */
  if (BURTC->FREEZE & BURTC_FREEZE_REGFREEZE_FREEZE)
  {
    return;
  }
  /* Check if mode is enabled */
  if ((BURTC->CTRL & _BURTC_CTRL_MODE_MASK) == BURTC_CTRL_MODE_DISABLE)
  {
    return;
  }

  while (BURTC->SYNCBUSY & BURTC_SYNCBUSY_COMP0) ;
}


/***************************************************************************//**
 * @brief Get BURTC compare value
 *
 * @param[in] comp Compare channel index value, must be 0 for Giant/Leopard.
 *
 * @return Currently configured value for this compare channel
 ******************************************************************************/
uint32_t BURTC_CompareGet(unsigned int comp)
{
  EFM_ASSERT(comp == 0);

  return BURTC->COMP0;
}


/***************************************************************************//**
 * @brief Reset counter
 *
 * @param[in] mode New mode of operation, after clearing
 ******************************************************************************/
void BURTC_CounterReset(void)
{
  /* Set and clear reset bit */
  BITBAND_Peripheral(&BURTC->CTRL, _BURTC_CTRL_RSTEN_SHIFT, 1);
  BITBAND_Peripheral(&BURTC->CTRL, _BURTC_CTRL_RSTEN_SHIFT, 0);
}


/***************************************************************************//**
 * @brief
 *   Restore BURTC to reset state
 * @note
 *   Before accessing the BURTC, BURSTEN in RMU->CTRL must be cleared.
 *   LOCK will not be reset to default value, as this will disable access
 *   to core BURTC registers.
 ******************************************************************************/
void BURTC_Reset(void)
{
  /* Verify RMU BURSTEN is disabled */
  EFM_ASSERT((RMU->CTRL & RMU_CTRL_BURSTEN) == 0);

  /* Restore all essential BURTC registers to default config */
  BURTC->CTRL      = _BURTC_CTRL_RESETVALUE;
  BURTC->IEN       = _BURTC_IEN_RESETVALUE;
  BURTC->LPMODE    = _BURTC_LPMODE_RESETVALUE;
  BURTC->LFXOFDET  = _BURTC_LFXOFDET_RESETVALUE;
  BURTC->POWERDOWN = _BURTC_POWERDOWN_RESETVALUE;
  BURTC->FREEZE    = _BURTC_FREEZE_RESETVALUE;
}


/** @} (end addtogroup BURTC) */
/** @} (end addtogroup EFM32_Library) */

#endif /* BURTC_PRESENT */
