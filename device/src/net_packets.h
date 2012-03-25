#ifndef __RESPIRENETWORK_NET_PACKETS_H
#define __RESPIRENETWORK_NET_PACKETS_H

#include <time.h>

struct net_packet_broadcast {
	uint8_t hello[5];
	time_t time;
} __attribute__((packed));

struct net_packet_rt {
	uint8_t hello[5];
	time_t time;
} __attribute__((packed));

#endif // __RESPIRENETWORK_NET_PACKETS_H
