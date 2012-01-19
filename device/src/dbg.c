#include <string.h>

#include "efm32.h"
#include "efm32_chip.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "dbg.h"

//#define DBG_ENABLE_LED

char debug_string[128];

void DBG_Init(void)
{
	uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
	uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
	uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;

	// Enable GPIO clock
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
	// Enable debug clock AUXHFRCO
	CMU->OSCENCMD |= CMU_OSCENCMD_AUXHFRCOEN;
	while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY))
		;

	// Enable Serial wire output pin
	GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

#if DEBUG_LOCATION == 0
	//Set location 0 (PF2)
	GPIO->ROUTE = (GPIO->ROUTE
			& ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;
	// Enable output on pin
	GPIO->P[4].MODEH &= ~(_GPIO_P_MODEL_MODE2_MASK);
	GPIO->P[4].MODEH |= GPIO_P_MODEL_MODE2_PUSHPULL;
#elif DEBUG_LOCATION == 1
	// Set location 1 (PC15)
	GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
	// Enable output on pin
	GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
	GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#else
#error "Invalid DEBUG_LOCATION"
#endif

	// Enable trace in core debug
	CoreDebug->DHCSR |= 1;
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

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
	/* Configure GPIO port 'DBG_LED_PORT' 'DBG_LED_PIN' as LED control outputs */
	/* Disable the LED by default */
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

