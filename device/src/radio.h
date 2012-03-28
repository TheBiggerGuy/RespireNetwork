#ifndef __RESPIRENETWORK_RADIO_H
#define __RESPIRENETWORK_RADIO_H

#include <stdint.h>
#include <stdbool.h>

#include "efm32.h"

#include "core_cm3.h"

#include "efm32_gpio.h"

#include "pins_and_ports.h"

#include "net_packets.h"

#define RADIO_CHANNEL 0x32

#define RADIO_CMD_R_REG        0x00
#define RADIO_CMD_W_REG        0x20
#define RADIO_CMD_R_RX_PAYLOAD 0x61
#define RADIO_CMD_W_TX_PAYLOAD 0xA0
#define RADIO_CMD_FLUSH_TX     0xE1
#define RADIO_CMD_FLUSH_RX     0xE2
#define RADIO_CMD_NOP          0xFF


#define RADIO_CONFIG      0x00
#define RADIO_EN_AA       0x01
#define RADIO_EN_RXADDR   0x02
#define RADIO_SETUP_AW    0x03
#define RADIO_SETUP_RETR  0x04
#define RADIO_RF_CH       0x05
#define RADIO_RF_SETUP    0x06
#define RADIO_STATUS      0x07
#define RADIO_RX_ADDR_P0  0x0A
#define RADIO_RX_ADDR_P1  0x0B
#define RADIO_TX_ADDR     0x10
#define RADIO_RX_PW_P0    0x11
#define RADIO_RX_PW_P1    0x12
#define RADIO_FIFO_STATUS 0x17
#define RADIO_FEATURE     0x1D

#define RADIO_CONFIG_MASK_RX_DR (1 << 6)
#define RADIO_CONFIG_MASK_RX_DS (1 << 5)
#define RADIO_CONFIG_MASK_RX_RT (1 << 4)
#define RADIO_CONFIG_EN_CRC     (1 << 3)
#define RADIO_CONFIG_CRCCO      (1 << 2)
#define RADIO_CONFIG_PWR_UP     (1 << 1)
#define RADIO_CONFIG_PRIM_RX    (1 << 0)
#define RADIO_CONFIG_DEFAULT    (RADIO_CONFIG_PRIM_RX | RADIO_CONFIG_PWR_UP | RADIO_CONFIG_CRCCO | RADIO_CONFIG_EN_CRC)
//#define RADIO_CONFIG_DEFAULT    (RADIO_CONFIG_PRIM_RX | RADIO_CONFIG_PWR_UP)

#define RADIO_EN_RXADDR_P5 (1 << 5)
#define RADIO_EN_RXADDR_P4 (1 << 4)
#define RADIO_EN_RXADDR_P3 (1 << 3)
#define RADIO_EN_RXADDR_P2 (1 << 2)
#define RADIO_EN_RXADDR_P1 (1 << 1)
#define RADIO_EN_RXADDR_P0 (1 << 0)

#define RADIO_SETUP_AW_3 0x01
#define RADIO_SETUP_AW_4 0x02
#define RADIO_SETUP_AW_5 0x03

#define RADIO_RF_SETUP_RF_DR_HIGH (1 << 3)

#define RADIO_STATUS_RX_DR   (1 << 6)
#define RADIO_STATUS_TX_DS   (1 << 5)
#define RADIO_STATUS_MAX_RT  (1 << 4)
#define RADIO_STATUS_RX_P_NO_MASK  0x0E
#define RADIO_STATUS_RX_P_NO_SHIFT 1

#define RADIO_FIFO_STATUS_TX_FULL  (1 << 4)
#define RADIO_FIFO_STATUS_TX_EMPTY (1 << 3)
#define RADIO_FIFO_STATUS_RES1     (1 << 3)
#define RADIO_FIFO_STATUS_RX_FULL  (1 << 1)
#define RADIO_FIFO_STATUS_RX_EMPTY (1 << 0)

#define RADIO_FEATURE_EN_DPL
#define RADIO_FEATURE_EN_ACK_PAY (1 << 1)
#define RADIO_FEATURE_EN_DYN_ACK (1 << 0)

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

struct radio_address {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
	uint8_t b4;
} __attribute__((packed));

typedef enum {
	Radio_Mode_RX,
	Radio_Mode_TX
} Radio_Modes_typdef;

void Radio_init(struct radio_address *local, struct radio_address *broadcast);

int Radio_send_broadcast(struct net_packet_broadcast *data);
int Radio_send_rt(struct net_packet_rt *data);

int Radio_loadbuf_broadcast(struct net_packet_broadcast *data);
int Radio_loadbuf_rt(struct net_packet_rt *data);

int Radio_available(void);
bool radio_has_packets_to_sent(void);

int Radio_recive(uint8_t* data, uint8_t maxLenght);

void Radio_setMode(Radio_Modes_typdef mode, bool powersycle);
void radio_set_parent(struct radio_address *parent);

void Radio_enable(bool state);

void Radio_deinit(void);

#endif // __RESPIRENETWORK_RADIO_H
