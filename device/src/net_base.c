#include <string.h>

#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"
#include "dbg.h"

#include "net_packets.h"
#include "net_base.h"

void net_base_rtc_callback(void);

void net_base_init(void){
	struct radio_address local;
	struct radio_address broadcast;

	//memcpy(&local, (void *) &(DEVINFO->UNIQUEL), 4);
	local.b0 = *(&(DEVINFO->UNIQUEL)+0);
	local.b1 = *(&(DEVINFO->UNIQUEL)+1);
	local.b2 = *(&(DEVINFO->UNIQUEL)+2);
	local.b3 = *(&(DEVINFO->UNIQUEL)+3);
	local.b4 = 0x00;

    broadcast.b0 = 0x55;
	broadcast.b1 = 0xAA;
	broadcast.b2 = 0xff;
	broadcast.b3 = 0xAA;
	broadcast.b4 = 0x55;

	// local and broadcast recive adresses and tx on local address
	Radio_init(&local, &broadcast);
}

bool net_base_is_sending = false;

void net_base_run(void) {
	struct net_packet_broadcast pkg;

	// every 1s
	RTC_set_irq(net_base_rtc_callback);

	while(true) {
		if(Radio_available() > 0) {
			Radio_recive((uint8_t *) &pkg, sizeof(struct net_packet_broadcast));
			pkg.time = 1;
		}
		if (net_base_is_sending) {
			while (radio_has_packets_to_sent());

			Radio_setMode(Radio_Mode_RX, false);
			Radio_enable(true);
			net_base_is_sending = false;
		}
	}

	// stop callback
	RTC_set_irq(NULL);
}

void net_base_rtc_callback(void) {
	struct net_packet_broadcast pkg;

	net_base_is_sending = true;

	Radio_enable(false);
	Radio_setMode(Radio_Mode_TX, false);

	pkg.hello[0] = 'h';
	pkg.hello[1] = 'e';
	pkg.hello[2] = 'l';
	pkg.hello[3] = 'l';
	pkg.hello[4] = 'o';

	Radio_loadbuf_broadcast(&pkg);
	Radio_enable(true);
	for(volatile int i=0; i < 150; ++i) {
		__NOP();
	}
	Radio_enable(false);
}

void net_base_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
