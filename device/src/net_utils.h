#ifndef __RESPIRENETWORK_NET_UTIL_H
#define __RESPIRENETWORK_NET_UTIL_H

#include <stdbool.h>

#include "radio.h"

bool net_address_equal(struct radio_address* a, struct radio_address* b);

#endif // __RESPIRENETWORK_NET_UTIL_H
