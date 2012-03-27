#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"

#include "net_packets.h"
#include "net_node.h"

void net_node_broadcast(void);
void net_node_tx(void);

struct radio_address broadcast;
struct radio_address local;

void net_node_init(void){

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
	Radio_init(&broadcast, &broadcast);


//	struct letimer_config letimer;
//	letimer.broadcast_period = 128;
//	letimer.broadcast_end = &net_node_broadcast;
//	letimer.wait = 0;
//	letimer.tx_period = 128;
//	letimer.tx_end = &net_node_tx;

	// every 1s on the second for 238ns
	//letimer_init(&letimer);

}

void net_node_broadcast(void){
	struct net_packet_broadcast packet;

	// convet to tx mode and load packet
	packet.time = RTC_getTime();
	Radio_loadbuf_broadcast(&packet);

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
