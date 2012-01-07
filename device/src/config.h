#ifndef __CONFIG_H
#define RESP_DEVKIT
#define NODE_ID 12

#define DEMO_MODE

#ifdef RESP_DEVICE
	#define RESP_ACCEL
	#define MMA_INT_PIN 2
	#define MMA_INT_PORT 0
#elif defined RESP_BLACK
	#define RESP_ACCEL
	#define MMA_INT_PIN 2
	#define MMA_INT_PORT 0
#elif defined RESP_DEVKIT
	#define RESP_ACCEL
	#define MMA_INT_PIN 6
	#define MMA_INT_PORT 3
#elif defined RESP_PRESSURE
	#define MMA_INT_PIN 2
	#define MMA_INT_PORT 0
#endif // RESP_DEVICE

#endif // __CONFIG_H
