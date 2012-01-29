#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "efm32.h"
#include "efm32_cmu.h"
#include "efm32_gpio.h"
#include "efm32_usart.h"

#include "main.h"
#include "radio.h"

void Radio_clearIRQ(void);
void Radio_setRxTxMode(bool tx);

uint8_t Radio_read_reg(uint8_t loc);
uint8_t Radio_read_payload(uint8_t* buffer);

void Radio_write_reg(uint8_t loc, uint8_t val);
void Radio_write_lreg(uint8_t loc, uint8_t val[5]);
void Radio_write_packet(uint8_t data[32]);

uint8_t lastStatus = 0;
uint8_t state = RADIO_STATE_POWER_DOWN;
bool dataReady = false;


#define PACKET_LEN 16

/**************************************************************************//**
 * @brief Setup a USART as SPI
 *****************************************************************************/
void Radio_init(void)
{
	/* Config the clocks ///////////////////////////////////////////////////// */
	// Enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO, true);
	// Enable USART clock
#if   RADIO_USART_NUM == 0
	CMU_ClockEnable(cmuClock_USART0, true);
#elif RADIO_USART_NUM == 1
	CMU_ClockEnable(cmuClock_USART1, true);
#elif RADIO_USART_NUM == 2
	CMU_ClockEnable(cmuClock_USART2, true);
#else
#error "Invalid 'RADIO_USART'"
#endif

	/* Config the USART ///////////////////////////////////////////////////// */
	// Setting baudrate
	RADIO_USART->CLKDIV = 128 * (SPI_PERCLK_FREQUENCY / SPI_BAUDRATE - 2);

	// Use synchronous (SPI) mode 3 with MSB first
	RADIO_USART->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF | USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLETRAILING;
	// Clear old transfers/receptions, and disabling interrupts
	RADIO_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
	RADIO_USART->IEN = 0;
	// Enable pins and setting location
	RADIO_USART->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
			| USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | (RADIO_LOCATION << 8);

	// Set to master and to control the CS line
	// Enabling Master, TX and RX
	RADIO_USART->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
	//RADIO_USART->CTRL |= USART_CTRL_AUTOCS;
	RADIO_USART->CTRL ^= USART_CTRL_AUTOCS;

	// Clear previous interrupts
	RADIO_USART->IFC = _USART_IFC_MASK;

	/* Config the IO pins /////////////////////////////////////////////////// */
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_TX,  gpioModePushPull, 0);  // MOSI
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_RX,  gpioModeInput, 0);     // MISO
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_CS,  gpioModePushPull, 0);  // CS
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_CLK, gpioModePushPull, 0); // Clock
	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_CE,  gpioModePushPull, 0); // CE

	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_IRQ, gpioModeInput, 0);    // IRQ

	GPIO_PinModeSet(RADIO_PORT, RADIO_PIN_IRQ+1, gpioModePushPull, 1);    // DBG
	Radio_DBG(false);

	/* Config Radio ///////////////////////////////////////////////////// */
	uint8_t addr[5];
	addr[0] = (0x00             >> 0)  & 0xff;
	addr[1] = (DEVINFO->UNIQUEL >> 0)  & 0xff;
	addr[2] = (DEVINFO->UNIQUEL >> 8)  & 0xff;
	addr[3] = (DEVINFO->UNIQUEL >> 16) & 0xff;
	addr[4] = (DEVINFO->UNIQUEL >> 24) & 0xff;
	Radio_write_lreg(RADIO_RX_ADDR_P0, addr); // set RX address 0
	Radio_write_reg(RADIO_RX_PW_P0, PACKET_LEN);

	addr[0] = (RADIO_BROADCAST_ADDR >> 0)  & 0xff;
	addr[1] = (RADIO_BROADCAST_ADDR >> 8)  & 0xff;
	addr[2] = (RADIO_BROADCAST_ADDR >> 16) & 0xff;
	addr[3] = (RADIO_BROADCAST_ADDR >> 24) & 0xff;
	addr[4] = (RADIO_BROADCAST_ADDR >> 32) & 0xff;
	Radio_write_lreg(RADIO_RX_ADDR_P1, addr); // set RX address 1
	Radio_write_reg(RADIO_RX_PW_P1, PACKET_LEN);

	Radio_write_lreg(RADIO_TX_ADDR, addr); // equal to RADIO_RX_ADDR_P1

	Radio_write_reg(RADIO_SETUP_RETR, 0x00); // disable auto retransmit
	Radio_write_reg(RADIO_RF_CH, RADIO_CHANNEL); // set RF channel

	/* Config IRQ //////////////////// */
	GPIO_IntConfig(RADIO_PORT, RADIO_PIN_IRQ, false, true, true);

	/* power up //////////////////////////////////////////////////// */
	Radio_setRxTxMode(false);
	Radio_clearIRQ();
	Radio_CE(true);
}

void GPIO_EVEN_IRQHandler(void) {
	if (GPIO_IntGet() & (1 << RADIO_PIN_IRQ)) {

		// read and clear radio irq
		uint8_t radioIRQ = Radio_read_reg(RADIO_CONFIG);
		Radio_clearIRQ();
		GPIO_IntClear(1 << RADIO_PIN_IRQ);

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
int Radio_send(uint8_t* data, uint8_t start, uint8_t length)
{
	uint8_t buf[PACKET_LEN];

	EFM_ASSERT(length <= PACKET_LEN);

	Radio_DBG(true);

	Radio_CE(false);
	Radio_setRxTxMode(true);

	memcpy(buf, data+(sizeof(uint8_t)*start), length);
	memset(buf+length, 0x00, PACKET_LEN-length); // TODO: set to 0xAA or 0x55

	Radio_write_packet(buf);

	Radio_DBG(false);
	Radio_CE(true);
	memset(buf, 0x00, PACKET_LEN); // time waste 10us
	memset(buf, 0xFF, PACKET_LEN); //
	memset(buf, 0x00, PACKET_LEN); // this 16byte 14Mhz is 5us per round
	Radio_DBG(true);
	Radio_CE(false);

	delay(1000);

	Radio_setRxTxMode(false);
	Radio_CE(true);

	Radio_DBG(false);

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

	EFM_ASSERT(length >= PACKET_LEN);

	if(!data) {
		buf = Radio_read_reg(RADIO_STATUS);
		if(! (buf & RADIO_CONFIG_MASK_RX_DR) ) {
			return 0;
		}
	}
	return Radio_read_payload(data);
}

/* private ///////////////////////////////////////////////////////////////// */

void Radio_setRxTxMode(bool tx)
{
	uint8_t val = RADIO_CONFIG_DEFAULT;

	if(tx) {
		val ^= RADIO_CONFIG_PRIM_RX;
	}
	Radio_write_reg(RADIO_CONFIG, val);
	delay(1);
}

void Radio_clearIRQ(void)
{
	Radio_write_reg(RADIO_STATUS, RADIO_STATUS_RX_DR | RADIO_STATUS_TX_DS | RADIO_STATUS_MAX_RT);
}


uint8_t Radio_read_reg(uint8_t loc)
{
	uint8_t buf;

	EFM_ASSERT(loc < 128);

	// clear RX buffer
	Radio_Rx_Clear();

	Radio_CS(false);
	// send cmd and NOP
	USART_Tx(RADIO_USART, RADIO_CMD_R_REG | loc);
	USART_Tx(RADIO_USART, RADIO_CMD_NOP);
	// read both status and cmd
	lastStatus = USART_Rx(RADIO_USART);
	buf = USART_Rx(RADIO_USART);
	Radio_CS(true);

	// save status
	lastStatus = buf & 0xff;

	// return cmd
	return buf;
}

uint8_t Radio_read_payload(uint8_t* buffer)
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	Radio_Rx_Clear();

	Radio_CS(false);
	// send cmd and NOP
	USART_Tx(RADIO_USART, RADIO_CMD_R_RX_PAYLOAD);
	USART_Tx(RADIO_USART, RADIO_CMD_NOP);
	// read both status and cmd
	lastStatus = USART_Rx(RADIO_USART);
	for(int i=0; i<PACKET_LEN; i++) {
		buffer[i] = USART_Rx(RADIO_USART);
		USART_Tx(RADIO_USART, RADIO_CMD_NOP);
	}
	// wait for spi to finish
	while (!(RADIO_USART->STATUS & USART_STATUS_TXC))
	Radio_CS(true);

	// clear RX buffer
	Radio_Rx_Clear();

	// return length
	return PACKET_LEN;
}

void Radio_write_reg(uint8_t loc, uint8_t val)
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	Radio_Rx_Clear();

	Radio_CS(false);
	// send cmd and NOP
	USART_Tx(RADIO_USART, RADIO_CMD_W_REG | loc);
	USART_Tx(RADIO_USART, val);
	// read both status and cmd
	lastStatus = USART_Rx(RADIO_USART);

	// wait for spi to finish
	while (!(RADIO_USART->STATUS & USART_STATUS_TXC))
	Radio_CS(true);

	// clear RX buffer
	Radio_Rx_Clear();
}

void Radio_write_lreg(uint8_t loc, uint8_t val[5])
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	Radio_Rx_Clear();

	Radio_CS(false);
	// send cmd and NOP
	USART_Tx(RADIO_USART, RADIO_CMD_W_REG | loc);
	for(uint8_t i=0; i<5; i++) {
		USART_Tx(RADIO_USART, val[i]);
	}
	// read both status and cmd
	lastStatus = USART_Rx(RADIO_USART);

	// wait for spi to finish
	while (!(RADIO_USART->STATUS & USART_STATUS_TXC))
	Radio_CS(true);

	// clear RX buffer
	Radio_Rx_Clear();
}

void Radio_write_packet(uint8_t data[32])
{
	EFM_ASSERT(loc < 128);

	// clear RX buffer
	Radio_Rx_Clear();

	Radio_CS(false);
	// send cmd and data
	USART_Tx(RADIO_USART, RADIO_CMD_W_TX_PAYLOAD);
	for(int i=0; i<32; i++) {
		USART_Tx(RADIO_USART, data[i]);
	}

	// wait for spi to finish
	while (!(RADIO_USART->STATUS & USART_STATUS_TXC));
	Radio_CS(true);

	// clear RX buffer
	Radio_Rx_Clear();
}

