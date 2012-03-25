#ifndef __RESPIRENETWORK_NET_BASE_H
#define __RESPIRENETWORK_NET_BASE_H

struct net_base_broadcast {
	uint8_t hello[5];
	time_t time;
};


void net_base_init(void);
void net_base_deinit(void);

#endif // __RESPIRENETWORK_NET_BASE_H
