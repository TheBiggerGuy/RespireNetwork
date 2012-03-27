#ifndef __RESPIRENETWORK_PINS_AND_PORTS_H
#define __RESPIRENETWORK_PINS_AND_PORTS_H

#include "efm32_gpio.h"

// RADIO SPI //////////////////////////////////////////////////////////////////
//
// Needed:
//  * SPI_USART_NUM - What USART number is the SPI on
//  * SPI_USART_LOC - Which location of the above is used
//
#define SPI_USART_NUM 1
#define SPI_USART_LOC 1

// Radio GPIO Pins ////////////////////////////////////////////////////////////
//
// Needed:
//  * RADIO_PORT_CE  - The port that the radio CE is on
//  * RADIO_PIN_CE   - The pin that the radio CE is on
//  * RADIO_PORT_IRQ - The port that the radio IRQ is on
//  * RADIO_PIN_IRQ  - The pin that the radio IRQ is on
//
#define RADIO_PORT_CE   gpioPortB
#define RADIO_PIN_CE    11
#define RADIO_PORT_IRQ  gpioPortD
#define RADIO_PIN_IRQ   5

// Debug //////////////////////////////////////////////////////////////////////
//
// Needed:
//  * DBG_LED_PORT - The port that the debug led or probe is on
//  * DBG_LED_PIN  - The pin that the debug led or probe is on
//  * DBG_SWO_LOC  - The location of the SWO
//
#define DBG_LED_PORT gpioPortD
#define DBG_LED_PIN  6
#define DBG_SWO_LOC  0

#endif // __RESPIRENETWORK_PINS_AND_PORTS_H
