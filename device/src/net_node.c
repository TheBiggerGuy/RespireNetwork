#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"

#include "net_base.h"
#include "net_node.h"

void net_node_broadcast(void);
void net_node_tx(void);

void net_node_init(void){
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
	Radio_init(&local, &broadcast, &local);


	struct letimer_config letimer;
	letimer.broadcast_period = 128;
	letimer.broadcast_end = &net_node_broadcast;
	letimer.wait = 0;
	letimer.tx_period = 128;
	letimer.tx_end = &net_node_tx;

	// every 1s on the second for 238ns
	letimer_init(&letimer);

}

void net_node_broadcast(void){
	struct net_base_broadcast packet;

	// convet to tx mode and load packet
	packet.time = RTC_getTime();
	Radio_loadbuf((uint8_t *) &packet, sizeof(packet));

	Radio_setMode(Radio_Mode_TX);
}

void net_node_tx(void){
	// convert back to rx mode
	Radio_setMode(Radio_Mode_RX);
}

void net_node_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
