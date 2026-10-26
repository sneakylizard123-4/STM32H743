#ifndef ETH_PHY_H
#define ETH_PHY_H

#include <stdint.h>

int eth_phy_test(void);
int eth_phy_read(uint8_t phy, uint8_t reg, uint16_t *val);

#endif
