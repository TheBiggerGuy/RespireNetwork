#include "dbg.h"
#include "radio.h"
#include "net_node.h"

void net_node_init(void){
	radio_address *local;
	radio_address *broadcast;

	local->b0 = *(&(DEVINFO->UNIQUEL)+0);
	local->b1 = *(&(DEVINFO->UNIQUEL)+1);
	local->b2 = *(&(DEVINFO->UNIQUEL)+2);
	local->b3 = *(&(DEVINFO->UNIQUEL)+3);
	local->b4 = 0x00;

	broadcast->b0 = 0xff;
	broadcast->b1 = 0xff;
	broadcast->b2 = 0xff;
	broadcast->b3 = 0xff;
	broadcast->b4 = 0xff;

	Radio_init(local, broadcast, local);
}

void net_node_send(void) {
	uint8_t datarx[32];
	uint8_t* datatx = (uint8_t*) "bob\x01\x02";

	memset(datarx, 0x00, sizeof(datarx));

	for (int i = 0; i < 5; i++) {
		//Radio_send(datatx, sizeof(datatx));
		if (Radio_available() > 0) {
			Radio_recive(datarx, sizeof(datarx));
			LOG_DEBUG("data: %s\n", datarx);
			break;
		}
	}
}

void net_node_deinit(void){
	Radio_deinit();
}
