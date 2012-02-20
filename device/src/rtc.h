#ifndef __RESPIRENETWORK_RTC_H
#define __RESPIRENETWORK_RTC_H

#include <time.h>

// User defined
#define RTC_PRESC CMU_LFAPRESC0_RTC_DIV1

// Computed defines
#define RTC_MAX_VALUE (1 << (RTC_PRESC + (24-16+1))) // TODO
#define RTC_1S (1 << 15) // TODO

// Prototypes
void RTC_init(void);
void RTC_deinit(void);

time_t RTC_getTime(void);
void RTC_setTime(time_t newTime);

#endif // __RESPIRENETWORK_RTC_H
