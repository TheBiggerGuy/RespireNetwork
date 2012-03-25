#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "efm32.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "rtc.h"
#include "radio.h"
#include "dbg.h"
#include "config.h"

volatile time_t baseTime = -1;

void RTC_init(void)
{
	// Config the clocks //////////////////////////////////////////////////////
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_RTC, true); // cmuClock_LFA enabled in the RTC command

	CMU_ClockEnable(cmuClock_CORELE, true);

	// lock RTC config update /////////////////////////////////////////////////
	// as it is in the low frequency area it is slow to write to
	RTC->FREEZE = RTC_FREEZE_REGFREEZE_FREEZE;

	// Config the RTC /////////////////////////////////////////////////////////

	CMU->FREEZE = CMU_FREEZE_REGFREEZE_FREEZE;
	CMU->LFAPRESC0 |= CMU_LFAPRESC0_RTC_DIV2; // DIV8192; //RTC_PRESC;
	CMU->FREEZE = CMU_FREEZE_REGFREEZE_UPDATE;
	while(CMU->SYNCBUSY & CMU_SYNCBUSY_LFAPRESC0);

	RTC->COMP0 = 2*RTC_S;

	// Enable interrupts //////////////////////////////////////////////////////
	RTC->IEN = RTC_IEN_OF | RTC_IEN_COMP0; // | RTC_IEN_COMP1;

	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);

	// enable RTC and set debug run
	RTC->CTRL = RTC_CTRL_EN;
#if defined(CONFIG_CLOCKS_ON_DEBUG)
	RTC->CTRL |= RTC_CTRL_DEBUGRUN;
#endif

	// Write the changes out and wait /////////////////////////////////////////
	RTC->FREEZE = RTC_FREEZE_REGFREEZE_UPDATE;
	while (RTC->SYNCBUSY)
		;
}

time_t RTC_getTime(void)
{
	if (baseTime < 0)
		return -1;
	return baseTime + (RTC->CNT >> RTC_S_SHIFT);
}

void RTC_setTime(time_t newTime)
{
	baseTime = newTime - (RTC->CNT >> RTC_S_SHIFT);
}

void RTC_IRQHandler(void)
{
	if (RTC->IF & RTC_IF_OF)
	{
		// RTC overflow
		if (baseTime > -1)
			baseTime += RTC_MAX_VALUE;
		RTC->IFC = RTC_IFC_OF;
	}
	if (RTC->IF & RTC_IF_COMP0)
	{
		RTC->COMP0 = (RTC->COMP0 + RTC_S) & 0xFFFFFF; // 24bit reg
		RTC->IFC = RTC_IFC_COMP0;
	}
}

void RTC_deinit(void)
{
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_DisableIRQ(RTC_IRQn);
	CMU_ClockEnable(cmuClock_RTC, false);
}

