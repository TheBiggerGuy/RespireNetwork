/*
 * config.h
 *
 *  Created on: 20 Feb 2012
 *      Author: Guy Taylor
 */

#ifndef __RESPIRENETWORK_CONFIG_H
#define __RESPIRENETWORK_CONFIG_H
#include <stdbool.h>

/**
 * Do the RTC and LETIMER continue clocking on debug stop
 *  * Defined   - They keep running
 *  * unDefined - They stop
 */
//#define CONFIG_CLOCKS_ON_DEBUG


#define HFRCO_FREQUENCY 14000000

#define CONFIG_FUNC_BASE 1
#define CONFIG_FUNC_NODE 2
#define CONFIG_FUNC_TEST 3

#define CONFIG_FUNC CONFIG_FUNC_TEST

#define CONFIG_DBG_ENABLE_PROBES

#endif // __RESPIRENETWORK_CONFIG_H
