#include <stdbool.h>

#include "radio.h"

bool net_address_equal(radio_address *a, radio_address *b){
	return (uint32_t) a == (uint32_t) b && a->b4 == b->b4;
}
