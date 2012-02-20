#include <stdbool.h>
#include <string.h>

#include "efm32.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "letimer.h"
#include "radio.h"
#include "dbg.h"
#include "pins_and_ports.h"
#include "config.h"

void(*letimer_tx_end)(void);

void letimer_init(uint16_t wait, uint16_t period, void(*tx_end)(void))
{
	// Config the clocks //////////////////////////////////////////////////////
	//CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_LETIMER0, true); // cmuClock_LFA enabled in the RTC command

	CMU_ClockEnable(cmuClock_CORELE, true);

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Config the IO pins /////////////////////////////////////////////////////
	GPIO_PinModeSet(LETIMER_PORT_CE, LETIMER_PIN_CE, gpioModePushPull, 0);

	// lock LETIMER config update (as it is in the low frequency area it is slow to write to)
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_FREEZE;

	letimer_tx_end = tx_end;

	// Config the LETIMER /////////////////////////////////////////////////////
	CMU->LFAPRESC0 |= CMU_LFACLKEN0_LETIMER0;
	LETIMER0->CTRL = LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_UFOA0_PWM | LETIMER_CTRL_RTCC1TEN | LETIMER_CTRL_REPMODE_ONESHOT;
#if defined(CONFIG_CLOCKS_ON_DEBUG)
	LETIMER0->CTRL |= LETIMER_CTRL_DEBUGRUN;
#endif

	//  + --> COMP0 ---> COMP1 ---> UNDERFLOW -+
	//  |                                      |
	//  +--<-------------------------------<---+
	LETIMER0->COMP1 = wait;          // wait
	LETIMER0->COMP0 = wait + period; // TOP

	LETIMER0->REP0 = 1;

	LETIMER0->ROUTE = LETIMER_ROUTE_LOCATION_LOC1 | LETIMER_ROUTE_OUT0PEN;

	// Enable interrupts //////////////////////////////////////////////////////
	LETIMER0->IEN = LETIMER_IEN_UF;

	NVIC_ClearPendingIRQ(LETIMER0_IRQn);
	NVIC_EnableIRQ(LETIMER0_IRQn);

	LETIMER0->CMD = LETIMER_CMD_STOP | LETIMER_CMD_CLEAR | LETIMER_CMD_CTO0;

	// Write the changes out and wait
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_UPDATE;
	while (LETIMER0->SYNCBUSY)
		;
}

void LETIMER0_IRQHandler(void)
{
	if (LETIMER0->IF & LETIMER_IF_UF)
	{
		// LETIMER0 underflow
		if (letimer_tx_end != NULL)
		{
			(*letimer_tx_end)();
		}
		LETIMER0->REP0 = 1;

		LETIMER0->IFC = LETIMER_IFC_UF;
	}
}

void letimer_deinit(void)
{
	NVIC_ClearPendingIRQ(LETIMER0_IRQn);
	NVIC_DisableIRQ(LETIMER0_IRQn);

	CMU_ClockEnable(cmuClock_LETIMER0, false);

	letimer_tx_end = NULL;
}
