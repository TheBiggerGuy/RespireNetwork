#include <string.h>

#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"

#include "net_base.h"

void net_base_end_rx(void);
void net_base_end_tx(void);

void net_base_init(void){
	radio_address local;
	radio_address broadcast;

	//memcpy(&local, (void *) &(DEVINFO->UNIQUEL), 4);
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
	letimer_init(4, &net_base_end_tx, 0xff, &net_base_end_rx, false);

}

void net_base_end_rx(void){
	struct net_base_broadcast packet;

	// convet to tx mode and load packet
	memcpy(packet.hello, "hello", 5);
	packet.time = RTC_getTime();
	Radio_loadbuf((uint8_t *) &packet, sizeof(packet));

	Radio_setMode(Radio_Mode_TX);
}

void net_base_end_tx(void){
	Radio_setMode(Radio_Mode_RX);
}

void net_base_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
