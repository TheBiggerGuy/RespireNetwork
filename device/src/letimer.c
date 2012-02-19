#include <stdbool.h>
#include <string.h>

#include "efm32.h"

#include "efm32_cmu.h"

#include "letimer.h"

void (*callback)(void) = NULL;

void letimer_init(uint16_t wait, uint16_t duration, void (*func)(void)){
	/* Config the clocks ////////////////////////////////////////////////// */
	//CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_LETIMER0, true); // cmuClock_LFA enabled in the RTC command

	CMU_ClockEnable(cmuClock_CORELE, true);

	// lock LETIMER config update (as it is in the low frequency area it is slow to write to)
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_FREEZE;

	/* Config the LETIMER ////////////////////////////////////////////// */
	CMU->LFAPRESC0 |= CMU_LFACLKEN0_LETIMER0;
	LETIMER0->CTRL = LETIMER_CTRL_DEBUGRUN | LETIMER_CTRL_COMP0TOP | LETIMER_CTRL_UFOA0_PWM | LETIMER_CTRL_RTCC0TEN | LETIMER_CTRL_REPMODE_ONESHOT;

	// set COMP1 to the total length of a cycle
	LETIMER0->COMP1 = wait;
	LETIMER0->COMP0 = wait + duration;

    LETIMER0->ROUTE = LETIMER_ROUTE_LOCATION_LOC0 | LETIMER_ROUTE_OUT0PEN;

	/* Enable interrupts /////////////////////////////////////// */
	LETIMER0->IEN = LETIMER_IEN_UF;

	NVIC_ClearPendingIRQ(LETIMER0_IRQn);
    NVIC_EnableIRQ(LETIMER0_IRQn);

    LETIMER0->CMD = LETIMER_CMD_STOP | LETIMER_CMD_CLEAR | LETIMER_CMD_CTO0;

    // Write the changes out and wait
	LETIMER0->FREEZE = LETIMER_FREEZE_REGFREEZE_UPDATE;
    while (LETIMER0->SYNCBUSY);

	callback = func;

	LETIMER0->CMD = LETIMER_CMD_START;
}

void LETIMER0_IRQHandler(void)
{
	if (LETIMER0->IF & LETIMER_IF_UF) {
		// LETIMER0 underlow
		if(callback != NULL){
			(*callback)();
		}
		LETIMER0->IFC = LETIMER_IFC_UF;
	}
}

void letimer_deinit(void){
	 CMU_ClockEnable(cmuClock_LETIMER0, false);
	 callback = NULL;
}
