#ifndef __RESPIRENETWORK_RTC_H
#define __RESPIRENETWORK_RTC_H

#include <time.h>

// User defined
#define RTC_PRESC CMU_LFAPRESC0_RTC_DIV32768

// Computed defines
#define RTC_MAX_VALUE (1 << (9+RTC_PRESC))

// Prototypes
void RTC_init(void);
void RTC_deinit(void);

time_t RTC_getTime(void);
void RTC_setTime(time_t newTime);

#endif // __RESPIRENETWORK_RTC_H
