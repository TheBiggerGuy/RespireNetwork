#ifndef __RESPIRENETWORK_NET_BASE_H
#define __RESPIRENETWORK_NET_BASE_H

void net_base_init(void);
void net_base_deinit(void);

void net_send(void);
void net_rx(void);
void net_tx(void);
void net_read_fifo(void);

#endif // __RESPIRENETWORK_NET_BASE_H
