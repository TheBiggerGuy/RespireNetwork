#ifndef __RESPIRENETWORK_LETIMER_H
#define __RESPIRENETWORK_LETIMER_H

#include <stdint.h>
#include <stdbool.h>

#include "efm32.h"

// User defined
#define LETIMER_PRESC CMU_LFAPRESC0_LETIMER0_DIV1

void letimer_init(uint16_t wait, void(*wait_end)(void));
void letimer_deinit(void);

void letimer_forcepin(bool state);
void letimer_forcepin_ish(void);

#endif // __RESPIRENETWORK_LETIMER_H
