#include "efm32.h"

#include "radio.h"
#include "letimer.h"
#include "rtc.h"
#include "net_base.h"

#include "net_recv_only.h"

void net_recv_only_init(void){
	struct radio_address local;
	struct radio_address broadcast;

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

	Radio_setMode(Radio_Mode_RX);
	Radio_enable(true);

}

int net_recive(uint8_t* data, uint8_t maxLenght){
	return Radio_recive(data, maxLenght);
}

int net_available(void) {
	return Radio_available();
}

void net_recv_only_deinit(void){
	Radio_deinit();
	letimer_deinit();
}
