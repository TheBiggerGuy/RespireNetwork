#ifndef __RESPIRENETWORK_RADIO_H
#define __RESPIRENETWORK_RADIO_H

#include <stdint.h>
#include <stdbool.h>

#include "efm32.h"

#include "core_cm3.h"

#include "efm32_gpio.h"


//#define RADIO_USART_NUM 1
//#define RADIO_LOCATION  1
#define RADIO_PORT      gpioPortD
//#define RADIO_PIN_TX    0
//#define RADIO_PIN_RX    1
//#define RADIO_PIN_CLK   2
//#define RADIO_PIN_CS    3
#define RADIO_PORT_CE   gpioPortB
#define RADIO_PIN_CE    11
#define RADIO_PORT_IRQ  gpioPortD
#define RADIO_PIN_IRQ   5

#define RADIO_CHANNEL 0x16

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

#define RADIO_CONFIG_MASK_RX_DR (1 << 6)
#define RADIO_CONFIG_MASK_RX_DS (1 << 5)
#define RADIO_CONFIG_MASK_RX_RT (1 << 4)
#define RADIO_CONFIG_EN_CRC     (1 << 3)
#define RADIO_CONFIG_CRCCO      (1 << 2)
#define RADIO_CONFIG_PWR_UP     (1 << 1)
#define RADIO_CONFIG_PRIM_RX    (1 << 0)
#define RADIO_CONFIG_DEFAULT    (RADIO_CONFIG_PRIM_RX | RADIO_CONFIG_PWR_UP | RADIO_CONFIG_CRCCO | RADIO_CONFIG_EN_CRC)

#define RADIO_STATUS_RX_DR   (1 << 6)
#define RADIO_STATUS_TX_DS   (1 << 5)
#define RADIO_STATUS_MAX_RT  (1 << 4)

#define RADIO_BROADCAST_ADDR 0x55AAFFAA55

#define RADIO_STATE_POWER_DOWN 1
#define RADIO_STATE_STANDBY    2
#define RADIO_STATE_RX         3
#define RADIO_STATE_TX         4


#if RADIO_PORT == gpioPortD
#if RADIO_PIN_IRQ == 0
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL0_PORTD
#elif RADIO_PIN_IRQ == 1
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL1_PORTD
#elif RADIO_PIN_IRQ == 2
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL2_PORTD
#elif RADIO_PIN_IRQ == 3
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL3_PORTD
#elif RADIO_PIN_IRQ == 4
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL4_PORTD
#elif RADIO_PIN_IRQ == 5
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL5_PORTD
#elif RADIO_PIN_IRQ == 6
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL6_PORTD
#elif RADIO_PIN_IRQ == 7
#define RADIO_IRQ_SELM GPIO_EXTIPSELL_EXTIPSEL7_PORTD
#else
#error "Invalid 'RADIO_PIN_IRQ'"
#endif
#else
#error "TODO"
#endif

#if RADIO_PIN_IRQ < 8
#define RADIO_IRQ_SEL GPIO->EXTIPSELL
#elif RADIO_PIN_IRQ < 16
#define RADIO_IRQ_SEL GPIO->EXTIPSELH
#else
#error "Invalid 'RADIO_PIN_IRQ'"
#endif

#if RADIO_PIN_IRQ % 2 == 0
#define RADIO_IRQH GPIO_EVEN_IRQn
#define RADIO_IRQHF GPIO_EVEN_IRQHandler
#else
#define RADIO_IRQH GPIO_ODD_IRQn
#define RADIO_IRQHF GPIO_ODD_IRQHandler
#endif

typedef struct {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
	uint8_t b4;
} radio_address;

typedef enum {
	Radio_Mode_RX,
	Radio_Mode_TX
} Radio_Modes_typdef;

void Radio_init(radio_address *local, radio_address *broadcast, radio_address *send);

int Radio_send(uint8_t* data, uint8_t length);

int Radio_loadbuf(uint8_t* data, uint8_t length);

int Radio_available(void);

int Radio_recive(uint8_t* data, uint8_t maxLenght);

void Radio_deinit(void);

#endif // __RESPIRENETWORK_RADIO_H
