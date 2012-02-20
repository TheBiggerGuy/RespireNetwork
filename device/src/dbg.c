#include <string.h>

#include "efm32.h"
#include "efm32_chip.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "dbg.h"

char debug_string[128];

/**
 * AUXCLK is a 14 MHz clock driven by a separate RC oscillator, AUXHFRCO.
 * This clock is used for flash programming and Serial Wire Output (SWO).
 *
 */
void DBG_init(void)
{
	uint32_t *dwt_ctrl       = (uint32_t *) 0xE0001000;
	uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
	uint32_t *tpiu_protocol  = (uint32_t *) 0xE00400F0;

	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Enable debug clock AUXHFRCO
	CMU_ClockEnable(cmuClock_DBG, true);

	// Enable Serial wire output pin
	GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

#if DBG_SWO_LOC == 0
	//Set location 0 (PF2)
	GPIO->ROUTE |= GPIO_ROUTE_SWLOCATION_LOC0;
	// Enable output on pin
	GPIO_PinModeSet(gpioPortF, 2,  gpioModePushPull, 0);
#elif DBG_SWO_LOC == 1
	// Set location 1 (PC15)
	GPIO->ROUTE |= GPIO_ROUTE_SWLOCATION_LOC1;
	// Enable output on pin
	GPIO_PinModeSet(gpioPortC, 15,  gpioModePushPull, 0);
#endif

	// Enable trace in core debug
	CoreDebug->DHCSR |= CoreDebug_DHCSR_C_DEBUGEN_Msk;
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk | CoreDebug_DEMCR_MON_EN_Msk;

	// Enable PC and IRQ sampling output
	*dwt_ctrl = 0x400113FF;
	// Set TPIU prescaler to 16
	*tpiu_prescaler = 0xf;
	// Set protocol to NRZ
	*tpiu_protocol = PROTOCOL_SERIALWIRE_NRZ;
	// Unlock ITM and output data
	ITM->LAR = ITM_UNLOCK_CODE;
	ITM->TCR = 0x10009;

	/* Guy Edit */
	//ITM->TPR = ITM_TPR_PRIVMASK_Msk & 0x03; // ITM Trace Privilege Register
	//ITM->TER = 0xFFFFFFFF; // ITM Trace Privilege Register
	/* End */

#ifdef DBG_ENABLE_LED
	GPIO_PinModeSet(DBG_LED_PORT, DBG_LED_PIN, gpioModePushPull, 0);
#endif
}

void DBG_LED_On(void)
{
#ifdef DBG_ENABLE_LED
	GPIO_PinOutSet(DBG_LED_PORT, DBG_LED_PIN);
#endif
}

void DBG_LED_Off(void)
{
#ifdef DBG_ENABLE_LED
	GPIO_PinOutClear(DBG_LED_PORT, DBG_LED_PIN);
#endif
}

void DBG_LED_Toggle(void)
{
#ifdef DBG_ENABLE_LED
	GPIO_PinOutToggle(DBG_LED_PORT, DBG_LED_PIN);
#endif
}

void DBG_deinit(void){
	// TODO
};

