#include <stdbool.h>
#include <string.h>

#include "efm32.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "letimer.h"
#include "radio.h"

struct letimer_config letimer_configuration;

void letimer_init(struct letimer_config *config)
{
	// Config the clocks //////////////////////////////////////////////////////
	//CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_LETIMER0, true); // cmuClock_LFA enabled in the RTC command

	CMU_ClockEnable(cmuClock_CORELE, true);

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Config the IO pins /////////////////////////////////////////////////////
	GPIO_PinModeSet(RADIO_PORT_CE, RADIO_PIN_CE, gpioModePushPull, 0);

	// lock LETIMER config update (as it is in the low frequency area it is slow to write to)
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_FREEZE;

	// Config the LETIMER /////////////////////////////////////////////////////
	CMU->LFAPRESC0 |= CMU_LFACLKEN0_LETIMER0;
	LETIMER0->CTRL = LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_UFOA0_TOGGLE
	        | LETIMER_CTRL_RTCC0TEN | LETIMER_CTRL_RTCC1TEN
	        | LETIMER_CTRL_REPMODE_ONESHOT; // | LETIMER_CTRL_DEBUGRUN;

	// set COMP1 to the total length of a cycle
	LETIMER0->COMP1 = 0; // wait; // first and only compare
	LETIMER0->COMP0 = 0xff; // wait + duration; // this is Top

	LETIMER0->REP0 = 1;
	LETIMER0->REP1 = 0;

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

	memcpy(&letimer_configuration, config, sizeof(struct letimer_config));

	LETIMER0->CMD = LETIMER_CMD_START;
}

void LETIMER0_IRQHandler(void)
{
	if (LETIMER0->IF & LETIMER_IF_UF)
	{
		// LETIMER0 underlow
		LETIMER0->REP0 = 1;
		if (letimer_configuration.broadcast_end != NULL && letimer_configuration._is_broadcast)
		{
			(*letimer_configuration.broadcast_end)();
		} else if (letimer_configuration.tx_end != NULL && !letimer_configuration._is_broadcast) {
			(*letimer_configuration.tx_end)();
		}
		letimer_configuration._is_broadcast = !letimer_configuration._is_broadcast;
		LETIMER0->IFC = LETIMER_IFC_UF;
	}
}

void letimer_deinit(void)
{
	NVIC_ClearPendingIRQ(LETIMER0_IRQn);
	NVIC_DisableIRQ(LETIMER0_IRQn);

	CMU_ClockEnable(cmuClock_LETIMER0, false);
	letimer_configuration.broadcast_end = NULL;
	letimer_configuration.tx_end = NULL;
}
