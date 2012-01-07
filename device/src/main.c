/**************************************************************************//**
 * @file
 * @brief Simple LCD blink demo for EFM32_Gxxx_STK
 * @author Energy Micro AS
 * @version 2.1.2
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
#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "efm32_chip.h"
#include "efm32_emu.h"
#include "efm32_gpio.h"
#include "efm32_i2c.h"
#include "efm32_usart.h"
#include "efm32_rtc.h"
#include "efm32_cmu.h"
#include "efm32_adc.h"

#include "config.h"
#include "NRF24.h"
#include "NRF24L01.h"
#include "nrf24_config.h"

#include "respire_common.h"


void IO_Init(void)
{
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    
    /* Enable LE clock and LFXO oscillator */
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;
    CMU->OSCENCMD |= CMU_OSCENCMD_LFXOEN;
    /* Wait until LFXO ready */
    /* Note that this could be done more energy friendly with an interrupt in EM1 */
    while (!(CMU->STATUS & CMU_STATUS_LFXORDY)) ;
    
    /* Select LFXO as clock source for LFACLK */
    CMU->LFCLKSEL = (CMU->LFCLKSEL & ~_CMU_LFCLKSEL_LFA_MASK) | CMU_LFCLKSEL_LFA_LFXO;
    
    /* Enable GPIO clock */
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
    
    /* To avoid false start, configure output US0_TX as high on PD0 */
    GPIO->P[3].DOUT |= (1 << 0);
    /* Pin PD0 is configured to Push-pull */
    GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE0_MASK) | GPIO_P_MODEL_MODE0_PUSHPULL;
    /* Pin PD1 is configured to Input enabled */
    GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE1_MASK) | GPIO_P_MODEL_MODE1_INPUT;
    /* Pin PD2 is configured to Push-pull */
    GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE2_MASK) | GPIO_P_MODEL_MODE2_PUSHPULL;
    /* To avoid false start, configure output NRF_CSN as high on PE13 */
    GPIO->P[3].DOUT |= (1 << 3);
    /* Pin PE13 is configured to Push-pull */
    GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE3_MASK) | GPIO_P_MODEL_MODE3_PUSHPULL;
    /* NRF_CE set to initial low */
    GPIO->P[3].DOUT &= ~(1 << 4);
    GPIO->P[3].MODEL = (GPIO->P[3].MODEL & ~_GPIO_P_MODEL_MODE4_MASK) | GPIO_P_MODEL_MODE4_PUSHPULL;
    
    /* Enable clock for RTC */
    CMU->LFACLKEN0 |= CMU_LFACLKEN0_RTC;
    
    /* Enable clock for USART1 */
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART1;
    usartInit.msbf = true;
    usartInit.clockMode = usartClockMode3;
    usartInit.baudrate = 7000000;
    USART_InitSync(USART1, &usartInit);
    //USART_Enable(USART1, usartEnable);
    USART1->ROUTE = (USART1->ROUTE & ~_USART_ROUTE_LOCATION_MASK) | USART_ROUTE_LOCATION_LOC1;
    /* Enable signals TX, RX, CLK, CS */
    USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;
    
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  uint8_t nrf_status;
  
  /* Chip errata */
  CHIP_Init();
  
  IO_Init();
  nrf_status = NRF_ReadRegister(NRF_FIFO_STATUS);
  nrf_status = NRF_Status();
  nrf_status = NRF_ReadRegister(NRF_STATUS);
  
  NRF_SetupTX();
  
  nrf_status = NRF_ReadRegister(NRF_FIFO_STATUS);
  while(nrf_status == nrf_status)
  {
    EMU_EnterEM2(true);
  }
}

