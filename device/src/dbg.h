#ifndef __RESPIRENETWORK_DBG_H
#define __RESPIRENETWORK_DBG_H

#include "efm32.h"

#include "efm32_gpio.h"

#include "config.h"
#include "pins_and_ports.h"

// This will check if the needed info is present when 'CONFIG_DBG_ENABLE_PROBES' is enabled
#if defined(CONFIG_DBG_ENABLE_PROBES)
#if !defined(DBG_PROBE_PORT)
#error "#if 'DBG_PROBE_PORT' needed for 'CONFIG_DBG_ENABLE_PROBES'"
#endif
#if !defined(DBG_PROBE_PIN0) || !defined(DBG_PROBE_PIN1) || !defined(DBG_PROBE_PIN2)
#error "#if 'DBG_PROBE_PINx' needed for 'CONFIG_DBG_ENABLE_PROBES'"
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

typedef enum {
	DBG_Probe_0,
	DBG_Probe_1,
	DBG_Probe_2
} DBG_Probe_typdef;

void DBG_probe_on(DBG_Probe_typdef probe);
void DBG_probe_off(DBG_Probe_typdef probe);
void DBG_probe_toggle(DBG_Probe_typdef probe);

#endif // __RESPIRENETWORK_DBG_H
