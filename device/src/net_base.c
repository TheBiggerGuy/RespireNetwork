#include <string.h>

#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"

#include "net_packets.h"
#include "net_base.h"

void net_base_end_rx(void);
void net_base_end_tx(void);

void net_base_init(void){
	struct radio_address n1;
	struct radio_address n2;
	struct radio_address broadcast;

	//memcpy(&local, (void *) &(DEVINFO->UNIQUEL), 4);
//	local.b0 = *(&(DEVINFO->UNIQUEL)+0);
//	local.b1 = *(&(DEVINFO->UNIQUEL)+1);
//	local.b2 = *(&(DEVINFO->UNIQUEL)+2);
//	local.b3 = *(&(DEVINFO->UNIQUEL)+3);
//	local.b4 = 0x00;
//
    broadcast.b0 = 0x55;
	broadcast.b1 = 0xAA;
	broadcast.b2 = 0xff;
	broadcast.b3 = 0xAA;
	broadcast.b4 = 0x55;

	n1.b0 = 0x37;
	n1.b1 = 0x37;
	n1.b2 = 0x37;
	n1.b3 = 0xe7;
	n1.b4 = 0x37;

	n2.b0 = 0xc2;
	n2.b1 = 0xc2;
	n2.b2 = 0xc2;
	n2.b3 = 0xc2;
	n2.b4 = 0xc2;

	// local and broadcast recive adresses and tx on local address
	Radio_init(&n2, &broadcast);
	radio_set_parent(&n1);

	// every 1s on the second for 238ns
	letimer_init(5, &net_base_end_tx, (1 << 14) -16, &net_base_end_rx, false);

}

void net_base_end_rx(void){
	struct net_packet_rt packet;

	// convet to tx mode and load packet
	memcpy(packet.hello, "hello", 5);
	packet.time = RTC_getTime();
	Radio_loadbuf_rt(&packet);

	Radio_setMode(Radio_Mode_TX);
}

void net_base_end_tx(void){
	Radio_setMode(Radio_Mode_RX);
}

void net_base_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
