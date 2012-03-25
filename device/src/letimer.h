#ifndef __RESPIRENETWORK_LETIMER_H
#define __RESPIRENETWORK_LETIMER_H

#include <stdint.h>

#include "efm32.h"

// User defined
#define LETIMER_PRESC CMU_LFAPRESC0_LETIMER0_DIV1

void letimer_init(uint16_t wait1, void(*wait1_end)(void), uint16_t wait2, void(*wait2_end)(void));
void letimer_deinit(void);

#endif // __RESPIRENETWORK_LETIMER_H
