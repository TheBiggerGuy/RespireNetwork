#include <stdbool.h>

#include "radio.h"

bool net_address_equal(struct radio_address *a, struct radio_address *b){
	return a->b0 == b->b0 && a->b1 == b->b1 && a->b2 == b->b2 && a->b3 == b->b3 && a->b4 == b->b4;
}
