#include <string.h>

#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"
#include "dbg.h"

#include "net_packets.h"
#include "net_node.h"

void net_node_end_tx(void);
void net_node_start_rx(void);

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
	Radio_init(&local, &broadcast);
}

void net_node_join(void){
	struct net_packet_broadcast pkg;

	// set up the radio
	Radio_setMode(Radio_Mode_RX, false);
	Radio_enable(true);

	// wait for becon
	while(Radio_available() == 0);

	// get the packet
	Radio_enable(false);
	Radio_recive((uint8_t *) &pkg, sizeof(struct net_packet_broadcast));

	RTC_reset_irq(115);
	letimer_init(100, NULL);
	RTC_set_irq(net_node_start_rx);
	DBG_LED_Off();
}

void net_node_run(void){
	struct net_packet_broadcast pkg;
	while(true) {
		while(Radio_available() == 0);
		Radio_recive((uint8_t *) &pkg, sizeof(struct net_packet_broadcast));
		Radio_enable(false);

		// convet to tx mode and load packet
		pkg.hello[0] = 'h';
		pkg.hello[1] = 'e';
		pkg.hello[2] = 'l';
		pkg.hello[3] = 'l';
		pkg.hello[4] = 'o';
		pkg.time = RTC_getTime();
		Radio_loadbuf_broadcast(&pkg);

		Radio_setMode(Radio_Mode_TX, false);
	}
}

void net_node_start_rx(void){
	DBG_LED_On();
	Radio_setMode(Radio_Mode_RX, true);
	Radio_enable(true);
}

void net_node_end_tx(void){
	Radio_setMode(Radio_Mode_RX, false);
	DBG_LED_Off();
}

void net_node_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
