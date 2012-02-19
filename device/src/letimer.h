#ifndef __RESPIRENETWORK_LETIMER_H
#define __RESPIRENETWORK_LETIMER_H

#include <stdint.h>
#include <stdbool.h>

struct letimer_config {
	uint16_t broadcast_period;
	void(*broadcast_end)(void);
	uint16_t wait;
	uint16_t tx_period;
	void(*tx_end)(void);
	bool _is_broadcast;
};

void letimer_init(struct letimer_config *config);
void letimer_deinit(void);

#endif // __RESPIRENETWORK_LETIMER_H
