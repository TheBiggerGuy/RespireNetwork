#ifndef __RESPIRENETWORK_DBG_H
#define __RESPIRENETWORK_DBG_H

#include "efm32.h"
#include "efm32_gpio.h"

/* TPIU protocal types */
#define PROTOCOL_TRACEPORT      0x00;
#define PROTOCOL_SERIALWIRE_MAN 0x01;
#define PROTOCOL_SERIALWIRE_NRZ 0x02;

/* ITM Lock Access Register - unlock code */
#define ITM_UNLOCK_CODE 0xC5ACCE55

#define DEBUG_LOCATION 0x00

void DBG_Init(void);

#define DBG_LED_PORT gpioPortC
#define DBG_LED_PIN 0

void DBG_LED_On(void);
void DBG_LED_Off(void);
void DBG_LED_Toggle(void);

#define LOG_DEBUG(x, y) printf(x, y)

#endif // __RESPIRENETWORK_DBG_H
