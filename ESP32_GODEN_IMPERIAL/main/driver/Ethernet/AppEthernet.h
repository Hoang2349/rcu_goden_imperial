#pragma once

#ifndef __APP_ETHERNET_H
#define __APP_ETHERNET_H

#include "stdbool.h"
// #include "common.h"

#define ETH_PHY_ADDR (1)
#define ETH_PHY_RST_GPIO (5)
#define ETH_MDC_GPIO (GPIO_NUM_23)
#define ETH_MDIO_GPIO (GPIO_NUM_18)
#define CONFIG_EXAMPLE_ETH_PHY_IP101 (1)
#define CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET (1)
#define MAX_LENGTH_IPCONFIG (20)
#define SIZE_MAC_ADDRESS (6)
void appEthernetInit(void);

#endif /* _APP_ETHERNET_ */
