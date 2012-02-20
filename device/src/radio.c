#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "efm32.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"

#include "main.h"
#include "radio.h"
#include "spi.h"
#include "dbg.h"


#define PACKET_LEN  16
#define ADDRESS_LEN 5

void Radio_clearIRQ(void);
void Radio_setMode(Radio_Modes_typdef mode);

uint8_t Radio_read_reg(uint8_t loc);
uint8_t Radio_read_payload(uint8_t* buffer);

void Radio_write_reg(uint8_t loc, uint8_t val);
void Radio_write_lreg(uint8_t loc, uint8_t val[ADDRESS_LEN]);
void Radio_write_packet(uint8_t data[PACKET_LEN]);

uint8_t state = RADIO_STATE_POWER_DOWN;
bool dataReady = false;

/**************************************************************************//**
 * @brief Setup a USART as SPI
 *****************************************************************************/
void Radio_init(radio_address *local, radio_address *broadcast, radio_address *send)
{
	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* Config the IO pins /////////////////////////////////////////////////// */
	GPIO_PinModeSet(RADIO_PORT_CE, RADIO_PIN_CE,  gpioModeInputPull, 0);  // CE
	GPIO_PinModeSet(RADIO_PORT_IRQ, RADIO_PIN_IRQ, gpioModeInput, 0);    // IRQ

	/* Enable the SPI ////////////////////////////////////////////////////// */
	spi_init();

	/* Config Radio /////////////////////////////////////////////////////// */
	Radio_write_lreg(RADIO_RX_ADDR_P0, (uint8_t *) local); // set RX address 0
	Radio_write_reg(RADIO_RX_PW_P0, PACKET_LEN);

	Radio_write_lreg(RADIO_RX_ADDR_P1, (uint8_t *) broadcast); // set RX address 1
	Radio_write_reg(RADIO_RX_PW_P1, PACKET_LEN);

	Radio_write_lreg(RADIO_TX_ADDR, (uint8_t *) send); // equal to RADIO_RX_ADDR_P1

	Radio_write_reg(RADIO_SETUP_RETR, 0x00); // disable auto retransmit
	Radio_write_reg(RADIO_RF_CH, RADIO_CHANNEL); // set RF channel

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
	Radio_setMode(Radio_Mode_RX);
	Radio_clearIRQ();
}

void RADIO_IRQHF(void) {
	if (GPIO->IF & (1 << RADIO_PIN_IRQ)) {

		// read and clear radio irq
		uint8_t radioIRQ = Radio_read_reg(RADIO_STATUS);
		Radio_clearIRQ();
		GPIO->IFC = (1 << RADIO_PIN_IRQ);

		if(radioIRQ & RADIO_CONFIG_MASK_RX_RT) {
			// Retransmitting
			__NOP();
		}

		if(radioIRQ & RADIO_CONFIG_MASK_RX_DS) {
			// Data sent
			__NOP();
		}

		if(radioIRQ & RADIO_CONFIG_MASK_RX_DR) {
			// Data ready to read
			dataReady = true;
		}

	}
}

/**************************************************************************//**
 * @brief Send a msg
 *****************************************************************************/
int Radio_send(uint8_t* data, uint8_t length)
{
	Radio_enable(false);
	Radio_setMode(Radio_Mode_TX);

	Radio_loadbuf(data, length);

	Radio_enable(true);
	delay(2);
	Radio_enable(false);

	Radio_setMode(Radio_Mode_RX);

	return length;
}

/**************************************************************************//**
 * @brief Send a msg
 *****************************************************************************/
int Radio_loadbuf(uint8_t* data, uint8_t length)
{
	uint8_t buf[PACKET_LEN];

	EFM_ASSERT(length <= PACKET_LEN);

	memcpy(buf, data, length);
	memset(buf+length, 0x00, PACKET_LEN-length); // TODO: set to 0xAA or 0x55

	Radio_write_packet(buf);

	return length;
}

/**************************************************************************//**
 * @brief How meany bytes are available
 *****************************************************************************/
int Radio_available(void) {
	if (dataReady) {
		return PACKET_LEN;
	} else {
		return 0;
	}
}

/**************************************************************************//**
 * @brief Read upto maxLenght bytes
 *****************************************************************************/
int Radio_recive(uint8_t* data, uint8_t maxLenght){
	uint8_t buf;

	EFM_ASSERT(maxLenght >= PACKET_LEN);

	if(!data) {
		buf = Radio_read_reg(RADIO_STATUS);
		if(! (buf & RADIO_CONFIG_MASK_RX_DR) ) {
			return 0;
		}
	}
	return Radio_read_payload(data);
}

void Radio_setMode(Radio_Modes_typdef mode)
{
	uint8_t val = RADIO_CONFIG_DEFAULT;

	if(mode == Radio_Mode_TX) {
		val ^= RADIO_CONFIG_PRIM_RX;
	}
	Radio_write_reg(RADIO_CONFIG, val);
	// TODO: may need delay
}

void Radio_enable(bool state)
{
	if (state) {
		GPIO->P[RADIO_PORT_CE].DOUTSET = 1 << RADIO_PIN_CE;
	} else {
		GPIO->P[RADIO_PORT_CE].DOUTCLR = 1 << RADIO_PIN_CE;
	}
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

uint8_t Radio_read_payload(uint8_t* buffer)
{
	uint8_t buf = RADIO_CMD_R_RX_PAYLOAD;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(&buf, 1);
	// read both status and cmd
	spi_read(buffer, 1, RADIO_CMD_NOP);
	spi_read(buffer, PACKET_LEN, RADIO_CMD_NOP);
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();

	// return length
	return PACKET_LEN;
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

void Radio_write_lreg(uint8_t loc, uint8_t val[ADDRESS_LEN])
{
	uint8_t buf = RADIO_CMD_W_REG | loc;

	EFM_ASSERT(loc < 128);

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and NOP
	spi_write(&buf, 1);
	spi_write(val, ADDRESS_LEN);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}

void Radio_write_packet(uint8_t data[PACKET_LEN])
{
	uint8_t buf = RADIO_CMD_W_TX_PAYLOAD;

	// clear RX buffer
	spi_clear_rx();

	spi_cs(false);
	// send cmd and data
	spi_write(&buf, 1);
	spi_write(data, PACKET_LEN);
	// wait for spi to finish
	spi_flush_tx();
	spi_cs(true);

	// clear RX buffer
	spi_clear_rx();
}
