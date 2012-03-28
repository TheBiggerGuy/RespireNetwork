#include <string.h>

#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"
#include "dbg.h"

#include "net_packets.h"
#include "net_base.h"

void net_base_end_rx(void);
void net_base_end_tx(void);

//#define TEST_TX_ONLY
//#define TEST_RX_ONLY
//#define TEST_RX_TX

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

#if defined(TEST_TX_ONLY)
	struct net_packet_broadcast p;
	Radio_setMode(Radio_Mode_TX);
	Radio_enable(true);
	p.hello[0] = 'h';
	p.hello[1] = 'e';
	p.hello[2] = 'l';
	p.hello[3] = 'l';
	p.hello[4] = 'o';
	while(true) {
		Radio_loadbuf_broadcast(&p);
		delay(200);
		Radio_loadbuf_broadcast(&p);
		delay(300);
		Radio_loadbuf_broadcast(&p);
		delay(100);
	}
#elif defined(TEST_RX_ONLY)
	Radio_setMode(Radio_Mode_RX);
	Radio_enable(true);
	while(true){
		if(Radio_available() > 0) {
			struct net_packet_broadcast buf;
			Radio_read_payload(&buf, sizeof(struct net_packet_broadcast));
			buf.time = 1;
		}
	}
#elif defined(TEST_RX_TX)
	struct net_packet_broadcast sent;
	struct net_packet_broadcast recv;
	sent.hello[0] = 'h';
	sent.hello[1] = 'e';
	sent.hello[2] = 'l';
	sent.hello[3] = 'l';
	sent.hello[4] = 'o';


	Radio_setMode(Radio_Mode_TX);
	Radio_loadbuf_broadcast(&sent);
	Radio_enable(true);
	for(volatile int i=0; i < 150; ++i) {
		__NOP();
	}
	Radio_enable(false);
	while (radio_has_packets_to_sent());

	while(true){
		Radio_setMode(Radio_Mode_RX);
		Radio_enable(true);
		while (Radio_available() == 0);
		DBG_LED_Toggle();
		Radio_recive((uint8_t *) &recv, sizeof(struct net_packet_broadcast));
		Radio_enable(false);
		Radio_setMode(Radio_Mode_TX);
		Radio_loadbuf_broadcast(&sent);
		Radio_enable(true);
		for(volatile int i=0; i < 150; ++i) {
			__NOP();
		}
		Radio_enable(false);
		while (radio_has_packets_to_sent());

	}
#endif

	// every 1s on the second for 238ns
	letimer_init(5, &net_base_end_tx, (1 << 14) -16, &net_base_end_rx, true);

}

void net_base_end_rx(void){
	struct net_packet_broadcast packet;

	// convert to tx mode and load packet
	memcpy(packet.hello, "hello", 5);
	packet.time = RTC_getTime();
	Radio_loadbuf_broadcast(&packet);

	Radio_setMode(Radio_Mode_TX);
}

void net_base_end_tx(void){
	Radio_setMode(Radio_Mode_RX);
}

void net_base_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
