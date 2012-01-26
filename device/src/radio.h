#ifndef __RESPIRENETWORK_RADIO_H
#define __RESPIRENETWORK_RADIO_H

#include <stdint.h>

void Radio_init(void);

int Radio_send(uint8_t* data, uint8_t start, uint8_t length);

int Radio_available(void);

int Radio_recive(uint8_t* data, uint8_t maxLenght);

#endif // __RESPIRENETWORK_RADIO_H
