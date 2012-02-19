#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"

#include "net_base.h"

struct net_base_broadcast {
	time_t time;
	uint8_t x;
};

void net_base_broadcast(void);

void net_base_init(void){
	radio_address local;
	radio_address broadcast;

	local.b0 = *(&(DEVINFO->UNIQUEL)+0);
	local.b1 = *(&(DEVINFO->UNIQUEL)+1);
	local.b2 = *(&(DEVINFO->UNIQUEL)+2);
	local.b3 = *(&(DEVINFO->UNIQUEL)+3);
	local.b4 = 0x00;

	broadcast.b0 = 0xff;
	broadcast.b1 = 0xff;
	broadcast.b2 = 0xff;
	broadcast.b3 = 0xff;
	broadcast.b4 = 0xff;

	// local and broadcast recive adresses and tx on local address
	Radio_init(&local, &broadcast, &broadcast);

	// every 1s on the second for 238ns
	letimer_init(1<<15, 0, 1<<7, net_base_broadcast);

}

void net_base_broadcast(void){
	struct net_base_broadcast packet;

	packet.time = RTC_getTime();
	packet.x = 33;

	Radio_loadbuf((uint8_t *) &packet, sizeof(packet));
}

void net_base_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
