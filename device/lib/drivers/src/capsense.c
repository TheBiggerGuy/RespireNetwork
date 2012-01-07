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

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/* EM header files */
#include "efm32.h"

/* Drivers */
#include "capsense.h"
#include "efm32_emu.h"
#include "efm32_acmp.h"

/* The current channel we are sensing */
static volatile uint8_t  currentChannel;
/* A bit vector which represents the channels to iterate through */
static const bool        channelsInUse[ACMP_CHANNELS] = { true, true, true, true, false, true, false, false };
/* This vector stores the latest read values from the ACMP */
static volatile uint32_t channelValues[ACMP_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
/* This stores the maximum values seen by a channel */
static volatile uint32_t channelMaxValues[ACMP_CHANNELS] = { 1, 1, 1, 1, 1, 1, 1, 1 };

/**************************************************************************//**
 * @brief TIMER0 interrupt handler.
 *        When TIMER0 expires the number of pulses on TIMER1 is inserted into
 *        channelValues. If this values is bigger than what is recorded in
 *        channelMaxValues, channelMaxValues is updated.
 *        Finally, the next ACMP channel is selected.
 *****************************************************************************/
void TIMER0_IRQHandler(void)
{
  uint32_t count;

  /* Stop timers */
  TIMER0->CMD = TIMER_CMD_STOP;
  TIMER1->CMD = TIMER_CMD_STOP;

  /* Clear interrupt flag */
  TIMER0->IFC = TIMER_IFC_OF;

  /* Read out value of TIMER1 */
  count = TIMER1->CNT;

  /* Store value in channelValues */
  channelValues[currentChannel] = count;

  /* Update channelMaxValues */
  if (count > channelMaxValues[currentChannel])
    channelMaxValues[currentChannel] = count;
}

/**************************************************************************//**
 * @brief Get the current channelValue for a channel
 * @return The channel.
 *****************************************************************************/
uint32_t CAPSENSE_getVal(uint8_t channel)
{
  return channelValues[channel];
}

/**************************************************************************//**
 * @brief Get the current normalized channelValue for a channel
 * @return The channel value in range (0-256).
 *****************************************************************************/
uint32_t CAPSENSE_getNormalizedVal(uint8_t channel)
{
  uint32_t max = channelMaxValues[channel];
  return (channelValues[channel] << 8) / max;
}

/**************************************************************************//**
 * @brief Get the state of the Gecko Button
 * @return true if the button is "pressed"
 *         false otherwise.
 *****************************************************************************/
bool CAPSENSE_getPressed(uint8_t channel)
{
  uint32_t treshold;
  /* Treshold is set to 12.5% below the maximum value */
  /* This calculation is performed in two steps because channelMaxValues is
   * volatile. */
  treshold  = channelMaxValues[channel];
  treshold -= channelMaxValues[channel] >> 3;

  if (channelValues[channel] < treshold)
  {
    return true;
  }
  return false;
}

/**************************************************************************//**
 * @brief Get the position of the slider
 * @return The position of the slider if it can be determined,
 *         -1 otherwise.
 *****************************************************************************/
int32_t CAPSENSE_getSliderPosition(void)
{
  int      i;
  int      minPos = -1;
  uint32_t minVal = 224; /* 0.875 * 256 */
  /* Values used for interpolation. There is two more which represents the edges.
   * This makes the interpolation code a bit cleaner as we do not have to make special
   * cases for handling them */
  uint32_t interpol[6] = { 255, 255, 255, 255, 255, 255 };

  /* The calculated slider position. */
  int position;

  /* Iterate through the 4 slider bars and calculate the current value divided by
   * the maximum value multiplied by 256.
   * Note that there is an offset of 1 between channelValues and interpol.
   * This is done to make interpolation easier.
   */
  for (i = 1; i < 5; i++)
  {
    /* interpol[i] will be in the range 0-256 depending on channelMax */
    interpol[i]  = channelValues[i - 1] << 8;
    interpol[i] /= channelMaxValues[i - 1];
    /* Find the minimum value and position */
    if (interpol[i] < minVal)
    {
      minVal = interpol[i];
      minPos = i;
    }
  }
  /* Check if the slider has not been touched */
  if (minPos == -1)
    return -1;

  /* Start position. Shift by 4 to get additional resolution. */
  /* Because of the interpol trick earlier we have to substract one to offset that effect */
  position = (minPos - 1) << 4;

  /* Interpolate with pad to the left */
  position -= ((256 - interpol[minPos - 1]) << 3)
              / (256 - interpol[minPos]);

  /* Interpolate with pad to the right */
  position += ((256 - interpol[minPos + 1]) << 3)
              / (256 - interpol[minPos]);

  return position;
}

/**************************************************************************//**
 * @brief This function iterates through all the capsensors and reads and
 *        initiates a reading. Uses EM1 while waiting for the result from 
 *        each sensor.
 *****************************************************************************/
void CAPSENSE_Sense(void)
{
  /* Use the default STK capacative sensing setup and enable it */
  ACMP_Enable(ACMP1);
  
  uint8_t ch;
  /* Iterate trough all channels */
  for (currentChannel = 0; currentChannel < ACMP_CHANNELS; currentChannel++)
  {
    /* If this channel is not in use, skip to the next one */
    if (!channelsInUse[currentChannel])
    {
      continue;
    }

    /* Set up this channel in the ACMP. */
    ch = currentChannel;
    ACMP_CapsenseChannelSet(ACMP1, (ACMP_Channel_TypeDef) ch);

    /* Reset timers */
    TIMER0->CNT = 0;
    TIMER1->CNT = 0;

    /* Start timers */
    TIMER0->CMD = TIMER_CMD_START;
    TIMER1->CMD = TIMER_CMD_START;  
  
    /* Wait for measurement to complete */
    EMU_EnterEM1();
  }
  
  /* Disable ACMP while not sensing to reduce power consumption */
  ACMP_Disable(ACMP1);
}

/**************************************************************************//**
 * @brief Initializes the capacative sense system.
 *        Capacative sensing uses two timers: TIMER0 and TIMER1 as well as ACMP.
 *        ACMP is set up in cap-sense (oscialltor mode).
 *        TIMER1 counts the number of pulses generated by ACMP1.
 *        When TIMER0 expires it generates an interrupt.
 *        The number of pulses counted by TIMER0 is then stored in channelValues
 *****************************************************************************/
void CAPSENSE_Init(void)
{ 
  /* Use the default STK capacative sensing setup */
  ACMP_CapsenseInit_TypeDef capsenseInit = ACMP_CAPSENSE_INIT_DEFAULT;
  
  /* Enable TIMER0, TIMER1, ACMP1 and PRS clock */
  CMU->HFPERCLKDIV |= CMU_HFPERCLKDIV_HFPERCLKEN;
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER0
                      | CMU_HFPERCLKEN0_TIMER1
                      | CMU_HFPERCLKEN0_ACMP1
                      | CMU_HFPERCLKEN0_PRS;

  /* Initialize TIMER0 - Prescaler 2^9, top value 10, interrupt on overflow */
  TIMER0->CTRL = TIMER_CTRL_PRESC_DIV512;
  TIMER0->TOP  = 10;
  TIMER0->IEN  = TIMER_IEN_OF;
  TIMER0->CNT  = 0;

  /* Initialize TIMER1 - Prescaler 2^10, clock source CC1, top value 0xFFFF */
  TIMER1->CTRL = TIMER_CTRL_PRESC_DIV1024 | TIMER_CTRL_CLKSEL_CC1;
  TIMER1->TOP  = 0xFFFF;

  /*Set up TIMER1 CC1 to trigger on PRS channel 0 */
  TIMER1->CC[1].CTRL = TIMER_CC_CTRL_MODE_INPUTCAPTURE /* Input capture      */
                       | TIMER_CC_CTRL_PRSSEL_PRSCH0   /* PRS channel 0      */
                       | TIMER_CC_CTRL_INSEL_PRS       /* PRS input selected */
                       | TIMER_CC_CTRL_ICEVCTRL_RISING /* PRS on rising edge */
                       | TIMER_CC_CTRL_ICEDGE_BOTH;    /* PRS on rising edge */

  /*Set up PRS channel 0 to trigger on ACMP1 output*/
  PRS->CH[0].CTRL = PRS_CH_CTRL_EDSEL_POSEDGE      /* Posedge triggers action */
                    | PRS_CH_CTRL_SOURCESEL_ACMP1  /* PRS source: ACMP1 */
                    | PRS_CH_CTRL_SIGSEL_ACMP1OUT; /* PRS source: ACMP1 */

  /* Set up ACMP1 in capsense mode */
  ACMP_CapsenseInit(ACMP1, &capsenseInit);

  /* Enable TIMER0 interrupt */
  NVIC_EnableIRQ(TIMER0_IRQn);
}

/** @} (end group Drivers) */

