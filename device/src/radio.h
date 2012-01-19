#ifndef __RESPIRENETWORK_RADIO_H
#define __RESPIRENETWORK_RADIO_H

#include <stdint.h>

void Radio_init(void);

int Radio_Send(uint8_t* data, uint8_t start, uint8_t length);

#endif // __RESPIRENETWORK_RADIO_H
