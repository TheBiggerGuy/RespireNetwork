#include <string.h>

#include "efm32.h"

#include "main.h"
#include "radio.h"

#include "net_packets.h"
#include "net_base.h"

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
	radio_set_parent(&broadcast);

	Radio_setMode(Radio_Mode_RX);
	Radio_enable(true);
}

void net_rx(void) {
	Radio_enable(false);
	Radio_setMode(Radio_Mode_RX);
	Radio_enable(true);
	delay(100);
}

void net_send(void) {
	struct net_packet_broadcast buf;
	memcpy(buf.hello, "Hello", 5);

	Radio_send_broadcast(&buf);
}

void net_read_fifo(void) {
	radio_read_fifo();
}

void net_base_deinit(void){
}
