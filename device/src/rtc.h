#ifndef __RESPIRENETWORK_RTC_H
#define __RESPIRENETWORK_RTC_H

#include <time.h>

#include "efm32.h"

// User defined
#define RTC_PRESC CMU_LFAPRESC0_RTC_DIV16384

// Computed defines
#define RTC_MAX_VALUE (1 << (RTC_PRESC + (24-16+1)))
#define RTC_S_SHIFT   8
#define RTC_S         (1 << RTC_S_SHIFT)

// Prototypes
void RTC_init(void);
void RTC_deinit(void);

time_t RTC_getTime(void);
void RTC_setTime(time_t newTime);

uint16_t RTC_getTickCount(void);

void RTC_set_irq(void(*irq)(void));
void RTC_reset_irq(int diff);

#endif // __RESPIRENETWORK_RTC_H
