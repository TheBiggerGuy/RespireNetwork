#ifndef __RESPIRENETWORK_RADIO_H
#define __RESPIRENETWORK_RADIO_H

#include <stdint.h>
#include <stdbool.h>

#include "core_cm3.h"

#include "efm32.h"
#include "efm32_gpio.h"


#define RADIO_USART_NUM 1
#define RADIO_LOCATION  1
#define RADIO_PORT      gpioPortD
#define RADIO_PIN_TX    0
#define RADIO_PIN_RX    1
#define RADIO_PIN_CLK   2
#define RADIO_PIN_CS    3
#define RADIO_PIN_CE    4
#define RADIO_PIN_IRQ   6

#define RADIO_CHANNEL 0x7F

/* Defines */
#define HFRCO_FREQUENCY         14000000
#define SPI_PERCLK_FREQUENCY    HFRCO_FREQUENCY
#define SPI_BAUDRATE            1000000

#define RADIO_CMD_R_REG        0x00
#define RADIO_CMD_W_REG        0x20
#define RADIO_CMD_R_RX_PAYLOAD 0x61
#define RADIO_CMD_W_TX_PAYLOAD 0xA0
#define RADIO_CMD_FLUSH_RX     0xE1
#define RADIO_CMD_FLUSH_TX     0xE2
#define RADIO_CMD_NOP          0xFF


#define RADIO_CONFIG     0x00
#define RADIO_EN_AA      0x01
#define RADIO_EN_RXADDR  0x02
#define RADIO_SETUP_AW   0x03
#define RADIO_SETUP_RETR 0x04
#define RADIO_RF_CH      0x05
#define RADIO_STATUS     0x07
#define RADIO_RX_ADDR_P0 0x0A
#define RADIO_RX_ADDR_P1 0x0B
#define RADIO_TX_ADDR    0x10
#define RADIO_RX_PW_P0   0x11
#define RADIO_RX_PW_P1   0x12

#define RADIO_CONFIG_PRIM_RX    0x01
#define RADIO_CONFIG_PWR_UP     0x02
#define RADIO_CONFIG_MASK_RX_RT 0x10
#define RADIO_CONFIG_MASK_RX_DR 0x20
#define RADIO_CONFIG_MASK_RX_DS 0x40
#define RADIO_CONFIG_DEFAULT 0x0C

#define RADIO_BROADCAST_ADDR 0x55AAFFAA55

#define RADIO_STATE_POWER_DOWN 1
#define RADIO_STATE_STANDBY    2
#define RADIO_STATE_RX         3
#define RADIO_STATE_TX         4


#if   RADIO_USART_NUM == 0
#define RADIO_USART USART0
#elif RADIO_USART_NUM == 1
#define RADIO_USART USART1
#elif RADIO_USART_NUM == 2
#define RADIO_USART USART2
#else
#error "Invalid 'RADIO_USART'"
#endif

void Radio_init(void);

int Radio_send(uint8_t* data, uint8_t start, uint8_t length);

int Radio_available(void);

int Radio_recive(uint8_t* data, uint8_t maxLenght);


static __INLINE void Radio_CE(bool state)
{
	if (state) {
		GPIO->P[RADIO_PORT].DOUTSET = 1 << RADIO_PIN_CE;
	} else {
		GPIO->P[RADIO_PORT].DOUTCLR = 1 << RADIO_PIN_CE;
	}
}

static __INLINE void Radio_CS(bool state) {
	if (state) {
		GPIO->P[RADIO_PORT].DOUTSET = 1 << RADIO_PIN_CS;
	} else {
		GPIO->P[RADIO_PORT].DOUTCLR = 1 << RADIO_PIN_CS;
	}
}

static __INLINE void Radio_DBG(bool state) {
	if (state) {
		GPIO->P[RADIO_PORT].DOUTSET = 1 << RADIO_PIN_IRQ+1;
	} else {
		GPIO->P[RADIO_PORT].DOUTCLR = 1 << RADIO_PIN_IRQ+1;
	}
}

#if defined(__GNUC__)
  #define __UNUSED __attribute__((unused))
#endif

static __INLINE void Radio_Rx_Clear(void)
{
	// Make sure compiler keeps it in and stops it warning
	volatile uint8_t buf __UNUSED;

	while ((RADIO_USART->STATUS & USART_STATUS_RXDATAV)) {
		buf = RADIO_USART->RXDATA;
	}
}

#endif // __RESPIRENETWORK_RADIO_H
