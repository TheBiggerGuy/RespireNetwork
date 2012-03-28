#ifndef __RESPIRENETWORK_NET_BASE_H
#define __RESPIRENETWORK_NET_BASE_H

#define NET_TEST_TX_ONLY 1
#define NET_TEST_RX_ONLY 2
#define NET_TEST_RX_TX   3

void net_test_init(uint8_t);
void net_test_deinit(void);

#endif // __RESPIRENETWORK_NET_BASE_H
