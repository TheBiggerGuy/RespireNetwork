#ifndef __RESPIRENETWORK_LETIMER_H
#define __RESPIRENETWORK_LETIMER_H

#include <stdint.h>

void letimer_init(uint16_t wait, uint16_t period, void(*tx_end)(void));
void letimer_deinit(void);

#endif // __RESPIRENETWORK_LETIMER_H
