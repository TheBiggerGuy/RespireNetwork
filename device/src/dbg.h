#ifndef __RESPIRENETWORK_DBG_H
#define __RESPIRENETWORK_DBG_H

#include "efm32.h"

#include "efm32_gpio.h"

#include "pins_and_ports.h"

/* Config */
#define DBG_ENABLE_LED

// This will check if the needed info is present when 'DBG_ENABLE_LED' is enabled
#if defined(DBG_ENABLE_LED)
#if !defined(DBG_LED_PORT)
#error "#if 'DBG_LED_PORT' needed for 'DBG_ENABLE_LED'"
#endif
#if !defined(DBG_LED_PIN)
#error "#if 'DBG_LED_PIN' needed for 'DBG_ENABLE_LED'"
#endif
#endif

// This will check if a valid location is given for the SWO
#if DBG_SWO_LOC < 0 || DBG_SWO_LOC > 2
#error "Invalid 'DBG_SWO_LOC' location"
#endif


/* TPIU protocal types */
#define PROTOCOL_TRACEPORT      0x00;
#define PROTOCOL_SERIALWIRE_MAN 0x01;
#define PROTOCOL_SERIALWIRE_NRZ 0x02;

/* ITM Lock Access Register - unlock code */
#define ITM_UNLOCK_CODE 0xC5ACCE55

void DBG_init(void);
void DBG_deinit(void);

void DBG_LED_On(void);
void DBG_LED_Off(void);
void DBG_LED_Toggle(void);

#endif // __RESPIRENETWORK_DBG_H
