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

void(*letimer_wait_end)(void) = NULL;

bool letimer_has_init_gpio = false;

void letimer_init(uint16_t wait, void(*wait_end)(void))
{
	// Config the clocks //////////////////////////////////////////////////////
	//CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_LETIMER0, true); // cmuClock_LFA enabled in the RTC command

	CMU_ClockEnable(cmuClock_CORELE, true);

	CMU_ClockEnable(cmuClock_GPIO, true);

	CMU->LFAPRESC0 |= CMU_LFAPRESC0_LETIMER0_DIV32;

	// Config the IO pins /////////////////////////////////////////////////////
	GPIO_PinModeSet(RADIO_PORT_CE, RADIO_PIN_CE, gpioModePushPull, 0);
	letimer_has_init_gpio = true;

	// lock LETIMER config update (as it is in the low frequency area it is slow to write to)
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_FREEZE;

	letimer_wait_end = wait_end;

	// Config the LETIMER /////////////////////////////////////////////////////
	CMU->LFAPRESC0 |= CMU_LFACLKEN0_LETIMER0;
	LETIMER0->CTRL = LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_REPMODE_ONESHOT | LETIMER_CTRL_UFOA0_PWM | LETIMER_CTRL_RTCC0TEN;
#if defined(CONFIG_CLOCKS_ON_DEBUG)
	LETIMER0->CTRL |= LETIMER_CTRL_DEBUGRUN;
#endif

	//       +---------+                +--------+
	//  ----/           \--------------/          \----------
	//
	//  + --> COMP0 ---> UNDERFLOW ---> COMP1 ---> UNDERFLOW --+
	//  |                                                      |
	//  +--<------------------------------------------------<--+
	LETIMER0->COMP0 = 2+wait;
	LETIMER0->COMP1 = 2;

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

void letimer_forcepin_ish(void){
	LETIMER0->CMD = LETIMER_CMD_CTO0;
}

void letimer_forcepin(bool state){
	if (!letimer_has_init_gpio) {
		GPIO_PinModeSet(RADIO_PORT_CE, RADIO_PIN_CE, gpioModePushPull, 0);
		letimer_has_init_gpio = true;
	}
	if (state) {
		GPIO->P[RADIO_PORT_CE].DOUTSET = 1 << RADIO_PIN_CE;
	} else {
		GPIO->P[RADIO_PORT_CE].DOUTCLR = 1 << RADIO_PIN_CE;
	}
}

void LETIMER0_IRQHandler(void)
{
	if (LETIMER0->IF & LETIMER_IF_UF) {
		// could be a long time so clear first
		LETIMER0->REP0 = 1;
		LETIMER0->IFC = LETIMER_IFC_UF;
		if (letimer_wait_end != NULL)
		{
			(*letimer_wait_end)();
		}
	}
}

void letimer_deinit(void)
{
	NVIC_ClearPendingIRQ(LETIMER0_IRQn);
	NVIC_DisableIRQ(LETIMER0_IRQn);

	LETIMER0->CMD = LETIMER_CMD_STOP;
	LETIMER0->ROUTE = 0x00;
	LETIMER0->CTRL = 0x00;

	letimer_wait_end = NULL;
}
