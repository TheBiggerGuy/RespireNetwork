#ifndef __RESPIRENETWORK_NET_RECV_ONLY_H
#define __RESPIRENETWORK_NET_RECV_ONLY_H

void net_recv_only_init(void);
void net_recv_only_deinit(void);

int net_recive(uint8_t* data, uint8_t maxLenght);
int net_available(void);

#endif // __RESPIRENETWORK_NET_NODE_H
