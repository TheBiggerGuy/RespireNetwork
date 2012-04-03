#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "efm32.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "main.h"
#include "radio.h"
#include "spi.h"
#include "dbg.h"
#include "letimer.h"
#include "rtc.h"

#include "net_packets.h"
#include "net_utils.h"

void Radio_clearIRQ(void);

uint8_t Radio_read_reg(uint8_t loc);
void Radio_write_reg(uint8_t loc, uint8_t val);

uint8_t Radio_read_lreg(uint8_t loc, uint8_t *buf, uint8_t length);
void Radio_write_lreg(uint8_t loc, uint8_t *val, uint8_t length);

void Radio_write_packet(uint8_t *data, uint8_t length);
uint8_t Radio_read_payload(uint8_t* buf, uint8_t length);

void Radio_flush(uint8_t buf);

bool radio_dataReadyToSend = false;
bool radio_dataReady = false;
bool radio_data_a_braodcast = true;
uint16_t radio_last_broadcast_tick = 0;

struct radio_address *radio_broadcast;
struct radio_address *radio_parent;
struct radio_address *radio_local;

/**************************************************************************//**
 * @brief Setup a USART as SPI
 *****************************************************************************/
void Radio_init(struct radio_address *local, struct radio_address *broadcast)
{
	struct radio_address comms_test;

	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Config the IO pins /////////////////////////////////////////////////// */
	GPIO_PinModeSet(RADIO_PORT_IRQ, RADIO_PIN_IRQ, gpioModeInput, 0);    // IRQ

	/* Make sure the radio is not broadcasting ///////////////////////////// */
	Radio_enable(false);

	/* Enable the SPI ////////////////////////////////////////////////////// */
	spi_init();

	/* power down to allow config */
	Radio_write_reg(RADIO_CONFIG, 0x00);
	delay(10);
	Radio_flush(RADIO_CMD_FLUSH_RX);
	Radio_flush(RADIO_CMD_FLUSH_TX);
	delay(2);

	/* Config Radio /////////////////////////////////////////////////////// */
	Radio_write_reg(RADIO_EN_AA,      0x00); // Disable all auto ack
	Radio_write_reg(RADIO_SETUP_RETR, 0x00); // disable auto retransmit

	Radio_write_reg(RADIO_EN_RXADDR,  RADIO_EN_RXADDR_P0 | RADIO_EN_RXADDR_P1); // enable only pipe 0 and 1
	Radio_write_reg(RADIO_SETUP_AW,   RADIO_SETUP_AW_5); // set address width
	Radio_write_reg(RADIO_RF_CH,      RADIO_CHANNEL); // set RF channel
	Radio_write_reg(RADIO_RF_SETUP,   RADIO_RF_SETUP_RF_DR_HIGH | (0x3 << 1)); // set RF speed (2Mbps) and full power

	Radio_write_lreg(RADIO_RX_ADDR_P0, (uint8_t *) local, 5); // set RX address 0
	Radio_write_reg(RADIO_RX_PW_P0, sizeof(struct net_packet_rt));
	radio_local = local;

	Radio_write_lreg(RADIO_RX_ADDR_P1, (uint8_t *) broadcast, 5); // set RX address 1
	Radio_write_reg(RADIO_RX_PW_P1, sizeof(struct net_packet_broadcast));
	radio_broadcast = broadcast;

	Radio_read_lreg(RADIO_RX_ADDR_P1, (uint8_t *) &comms_test, 5);
	if (!net_address_equal(broadcast, &comms_test)) {
		exit(EXIT_FAILURE);
	}

	/* Config IRQ //////////////////// */
	// select the port and pin
	RADIO_IRQ_SEL |= RADIO_IRQ_SELM;
	// Enable falling edge (this enables on all same numbered pins accross the ports)
    GPIO->EXTIFALL |= 1 << RADIO_PIN_IRQ;
    // Clear any pending interrupt
    GPIO->IFC = 1 << RADIO_PIN_IRQ;
    // Enable
    GPIO->IEN = 1 << RADIO_PIN_IRQ;
    NVIC_ClearPendingIRQ(RADIO_IRQH);
	NVIC_EnableIRQ(RADIO_IRQH);

	/* power up //////////////////////////////////////////////////// */
	Radio_flush(RADIO_CMD_FLUSH_RX);
	Radio_flush(RADIO_CMD_FLUSH_TX);
	Radio_clearIRQ();
	Radio_setMode(Radio_Mode_RX, false); // powers up
	delay(10);
}

void RADIO_IRQHF(void) {
	uint8_t status;

	DBG_probe_on(DBG_Probe_2);
	if (GPIO->IF & (1 << RADIO_PIN_IRQ)) {

		// read and clear radio irq
		status = Radio_read_reg(RADIO_STATUS);

		if(status & RADIO_STATUS_MAX_RT) {
			// Retransmitting
			__NOP();
		}

		if(status & RADIO_STATUS_TX_DS) {
			// Data sent
			radio_dataReadyToSend = false;
			__SEV(); // wake up process that may be waiting for this
		}

		if(status & RADIO_STATUS_RX_DR) {
			// Data ready to read
			uint8_t pipe;

			radio_dataReady = true;
			bool isEmpty = false;
			while(!isEmpty) {
				pipe   = (Radio_read_reg(RADIO_STATUS) & RADIO_STATUS_RX_P_NO_MASK) >> RADIO_STATUS_RX_P_NO_SHIFT;
				if (pipe == 0x00) {
					radio_data_a_braodcast = false;
				} else {
					radio_last_broadcast_tick = RTC_getTickCount();
					radio_data_a_braodcast = true;
				}
				isEmpty = (Radio_read_reg(RADIO_FIFO_STATUS) & RADIO_FIFO_STATUS_RX_EMPTY) == RADIO_FIFO_STATUS_RX_EMPTY;
			}
			// Radio_flush(RADIO_CMD_FLUSH_RX); // TODO
			__SEV(); // wake up process that may be waiting for this
		}

		Radio_clearIRQ();
		GPIO->IFC = (1 << RADIO_PIN_IRQ);
		DBG_probe_off(DBG_Probe_2);
	}
}

/**************************************************************************//**
 * @brief Send a msg
 *****************************************************************************/
//int Radio_send_broadcast(struct net_packet_broadcast *data)
//{
//	Radio_enable(false);
//	Radio_setMode(Radio_Mode_TX);
//
//	Radio_loadbuf_broadcast(data);
//
//	Radio_enable(true);
//	delay(20);
//	Radio_enable(false);
//
//	Radio_setMode(Radio_Mode_RX);
//
//	return sizeof(struct net_packet_broadcast);
//}
//
//int Radio_send_rt(struct net_packet_rt *data)
//{
//	Radio_enable(false);
//	Radio_setMode(Radio_Mode_TX);
//
//	Radio_loadbuf_rt(data);
//
//	Radio_enable(true);
//	delay(2);
//	Radio_enable(false);
//
//	Radio_setMode(Radio_Mode_RX);
//
//	return sizeof(struct net_packet_rt);
//}

/**************************************************************************//**
 * @brief Send a msg
 *****************************************************************************/
int Radio_loadbuf_broadcast(struct net_packet_broadcast *data)
{
	Radio_write_lreg(RADIO_TX_ADDR, (uint8_t *) radio_broadcast, sizeof(struct radio_address)); // equal to RADIO_RX_ADDR_P1
	Radio_write_packet((uint8_t *) data, sizeof(struct net_packet_broadcast));

	radio_dataReadyToSend = true;
	return sizeof(struct net_packet_broadcast);
}

int Radio_loadbuf_rt(struct net_packet_rt *data)
{
	Radio_write_lreg(RADIO_TX_ADDR, (uint8_t *) radio_parent, sizeof(struct radio_address)); // equal to RADIO_RX_ADDR_P1
	Radio_write_packet((uint8_t *) data, sizeof(struct net_packet_rt));

	radio_dataReadyToSend = true;
	return sizeof(struct net_packet_rt);
}

/**************************************************************************//**
 * @brief How meany bytes are available
 *****************************************************************************/
int Radio_available(void) {
	if (radio_dataReady) {
		if (radio_data_a_braodcast) {
			return sizeof(struct net_packet_broadcast);
		} else {
			return sizeof(struct net_packet_rt);
		}
	} else {
		return 0;
	}
}

bool radio_has_packets_to_sent(void) {
	return radio_dataReadyToSend;
}

/**************************************************************************//**
 * @brief Read upto maxLenght bytes
 *****************************************************************************/
int Radio_recive(uint8_t* data, uint8_t maxLenght){
	if(!radio_dataReady) {
		return 0;
	}
	return Radio_read_payload(data, maxLenght);
}

void Radio_setMode(Radio_Modes_typdef mode, bool powersycle)
{
	uint8_t val = RADIO_CONFIG_DEFAULT;

	if (powersycle) {
		Radio_write_reg(RADIO_CONFIG, 0x00);
	}

	if(mode == Radio_Mode_TX) {
		val = val & ~RADIO_CONFIG_PRIM_RX; // disable RX
	} else {
		val = val | RADIO_CONFIG_PRIM_RX; // enable RX
	}
	//Radio_enable(false);
	//Radio_flush(RADIO_CMD_FLUSH_RX);
	//Radio_flush(RADIO_CMD_FLUSH_TX);
	Radio_write_reg(RADIO_CONFIG, val);
	//Radio_enable(true);
	// TODO: may need delay
}

void radio_set_parent(struct radio_address *parent){
	radio_parent = parent;
}

void Radio_enable(bool state)
{
	letimer_forcepin(state);
}

uint16_t radio_get_last_broadcast_time(void) {
	return radio_last_broadcast_tick;
}

void Radio_deinit(void) {
	Radio_enable(false);
	spi_deinit();
}

/* private ///////////////////////////////////////////////////////////////// */

void Radio_clearIRQ(void)
{
	Radio_write_reg(RADIO_STATUS, RADIO_STATUS_RX_DR | RADIO_STATUS_TX_DS | RADIO_STATUS_MAX_RT);
}


uint8_t Radio_read_reg(uint8_t loc)
{
	uint8_t buf[2];

	EFM_ASSERT(loc < 128);

	buf[0] = RADIO_CMD_R_REG | loc;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd
	spi_write(buf, 1);
	// read both status and cmd
	spi_read(buf, 2, RADIO_CMD_NOP);
	spi_cs(true);

	// return reg
	return buf[1];
}

uint8_t Radio_read_lreg(uint8_t loc, uint8_t* buf, uint8_t length)
{
	EFM_ASSERT(loc < 128);

	buf[0] = RADIO_CMD_R_REG | loc;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(buf, 1);
	// read both status and cmd
	spi_read(buf, 1, RADIO_CMD_NOP);
	spi_read(buf, length, RADIO_CMD_NOP);
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();

	// return length
	return length;
}

uint8_t Radio_read_payload(uint8_t* buf, uint8_t length)
{
	buf[0] = RADIO_CMD_R_RX_PAYLOAD;

	if (length > 32) {
		length = 32; // max length is 32
	}

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(buf, 1);
	// read both status and cmd
	spi_read(buf, 1, RADIO_CMD_NOP);
	spi_read(buf, length, RADIO_CMD_NOP);
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();

	radio_dataReady = false;
	// return length
	return length;
}

void Radio_write_reg(uint8_t loc, uint8_t val)
{
	uint8_t buf[2];

	EFM_ASSERT(loc < 128);

	// Generate the packet
	buf[0] = RADIO_CMD_W_REG | loc;
	buf[1] = val;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(buf, 2);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}

void Radio_write_lreg(uint8_t loc, uint8_t *val, uint8_t length)
{
	uint8_t buf = RADIO_CMD_W_REG | loc;

	EFM_ASSERT(loc < 128);

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(&buf, 1);
	spi_write(val, length);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}

void Radio_write_packet(uint8_t *data, uint8_t length)
{
	uint8_t buf = RADIO_CMD_W_TX_PAYLOAD;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and data
	spi_write(&buf, 1);
	spi_write(data, length);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}

void Radio_flush(uint8_t buf)
{
	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(&buf, 1);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}
