#ifndef __RESPIRENETWORK_SPI_H
#define __RESPIRENETWORK_SPI_H

#include <stdint.h>

#include "efm32.h"

#include "efm32_gpio.h"
#include "efm32_cmu.h"

#include "pins_and_ports.h"
#include "config.h"

/* Defines */
#define SPI_PERCLK_FREQUENCY    HFRCO_FREQUENCY
#define SPI_BAUDRATE            7000000

/* Auto generated defines from above info */
#if SPI_USART_NUM == 0
/* USART Number 0 */

#define SPI_USART USART0
#define SPI_CLOCK cmuClock_USART0

#error "TODO: SPI ports"

#elif SPI_USART_NUM == 1
/* USART Number 1 */

#define SPI_USART USART1
#define SPI_CLOCK cmuClock_USART1
#define SPI_IRQ_RX  USART1_RX_IRQHandler
#define SPI_IRQ_RXN USART1_RX_IRQn
#define SPI_IRQ_TX  USART1_TX_IRQHandler
#define SPI_IRQ_TXN USART1_TX_IRQn

#if SPI_USART_LOC == 0
#error "TODO: SPI ports"
#elif SPI_USART_LOC == 1
#define SPI_PORT     gpioPortD
#define SPI_PIN_TX   0
#define SPI_PIN_RX   1
#define SPI_PIN_CLK  2
#define SPI_PIN_CS   3
#else
#error "TODO: SPI ports"
#endif

#elif SPI_USART_NUM == 2
/* USART Number 2 */

#define SPI_USART USART2
#define SPI_CLOCK cmuClock_USART2
#error "TODO: SPI ports"

#else
#error "Invalid 'SPI_USART_NUM'"
#endif

#if SPI_USART_LOC == 0
#define SPI_ROUTE_LOC USART_ROUTE_LOCATION_LOC0
#elif SPI_USART_LOC == 1
#define SPI_ROUTE_LOC USART_ROUTE_LOCATION_LOC1
#elif SPI_USART_LOC == 2
#define SPI_ROUTE_LOC USART_ROUTE_LOCATION_LOC2
#elif SPI_USART_LOC == 3
#define SPI_ROUTE_LOC USART_ROUTE_LOCATION_LOC3
#else
#error "Invalid 'SPI_USART_LOC'"
#endif

#if (SPI_BAUDRATE*2) > HFRCO_FREQUENCY
#error "Invalid 'SPI_BAUDRATE'"
#endif

/* Prototypes */
void spi_init(void);
int spi_read(uint8_t* buffer, int lenght, uint8_t flush_byte);
int spi_write(uint8_t* buffer, int length);
void spi_deinit(void);
void spi_cs(bool enable);
void spi_clear_rx(void);
void spi_clear_tx(void);
void spi_flush_tx(void);

#endif // __RESPIRENETWORK_SPI_H
