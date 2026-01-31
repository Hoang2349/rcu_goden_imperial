/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "AppEthernet.h"

#include <stdio.h>
#include <string.h>

#include "AppGpio.h"
#include "AppSpiffs.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "ethernet_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

/**
 * @brief Event handler for Ethernet events connect/disconect
 * @param void *arg Not handle
 * @param esp_event_base_t eventBase Not handle
 * @param int32_t eventId id event, each event corresponds to each ID
 * @param void *eventData Includes event data
 */
static void ethEventHandler(void *arg, esp_event_base_t eventBase,
                            int32_t eventId, void *eventData)
{
    uint8_t macAddress[SIZE_MAC_ADDRESS] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t ethHandle = *(esp_eth_handle_t *)eventData;

    switch (eventId)
    {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(ethHandle, ETH_CMD_G_MAC_ADDR, macAddress);
            ESP_LOGI(__FUNCTION__, "Ethernet Link Up");
            ESP_LOGI(__FUNCTION__,
                     "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                     macAddress[0], macAddress[1], macAddress[2], macAddress[3],
                     macAddress[4], macAddress[5]);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(__FUNCTION__, "Ethernet Link Down");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI(__FUNCTION__, "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGI(__FUNCTION__, "Ethernet Stopped");
            break;
        default:
            break;
    }
}

/**
 * @brief Event handler for IP_EVENT_ETH_GOT_IP
 * @param void *arg Not handle
 * @param esp_event_base_t eventBase Not handle
 * @param int32_t eventId id event, each event corresponds to each ID
 * @param void *eventData Includes event data
 */
static void gotIpEventHandler(void *arg, esp_event_base_t eventBase,
                              int32_t eventId, void *eventData)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)eventData;
    const esp_netif_ip_info_t *ipInfo = &event->ip_info;

    ESP_LOGI(__FUNCTION__, "Ethernet Got IP Address");
    ESP_LOGI(__FUNCTION__, "~~~~~~~~~~~");
    ESP_LOGI(__FUNCTION__, "ETHIP:" IPSTR, IP2STR(&ipInfo->ip));
    ESP_LOGI(__FUNCTION__, "ETHMASK:" IPSTR, IP2STR(&ipInfo->netmask));
    ESP_LOGI(__FUNCTION__, "ETHGW:" IPSTR, IP2STR(&ipInfo->gw));
    ESP_LOGI(__FUNCTION__, "~~~~~~~~~~~");
}

/**
 * @brief Init drived Ethernet and config static IP
 */
void appEthernetInit(void)
{
    uint8_t ethPortCount = 0;
    esp_eth_handle_t *ethHandles;
    ESP_ERROR_CHECK(example_eth_init(&ethHandles, &ethPortCount));
    esp_netif_config_t netifConfig = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *espNetif = esp_netif_new(&netifConfig);
    uint32_t ip = 0;
    uint32_t gateway = 0;
    uint32_t netmask = 0;
    if (getStaticIp(&ip, &gateway, &netmask))
    {
        // stop dhcp
        if (esp_netif_dhcpc_stop(espNetif) != ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Failed to stop dhcp client");
            goto RunDhcp;
        }

        // set static IP
        esp_netif_ip_info_t staticIp;
        memset(&staticIp, 0, sizeof(esp_netif_ip_info_t));
        staticIp.ip.addr = ip;
        staticIp.gw.addr = gateway;
        staticIp.netmask.addr = netmask;

        if (esp_netif_set_ip_info(espNetif, &staticIp) != ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Failed to set ip info");
            esp_netif_dhcpc_start(espNetif);
            goto RunDhcp;
        }

        // set DNS
        esp_netif_dns_info_t staticDns;
        staticDns.ip.u_addr.ip4.addr = esp_ip4addr_aton("8.8.8.8");
        staticDns.ip.type = ESP_IPADDR_TYPE_V4;
        if (esp_netif_set_dns_info(espNetif, ESP_NETIF_DNS_MAIN, &staticDns) !=
            ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Failed to set DNS ethernet");
            esp_netif_dhcpc_start(espNetif);
        }
    RunDhcp:
        ESP_ERROR_CHECK(esp_event_handler_register(
            IP_EVENT, IP_EVENT_ETH_GOT_IP, &gotIpEventHandler, NULL));
    }
    else
    {
        ESP_ERROR_CHECK(esp_event_handler_register(
            IP_EVENT, IP_EVENT_ETH_GOT_IP, &ethEventHandler, NULL));
    }

    ESP_ERROR_CHECK(
        esp_netif_attach(espNetif, esp_eth_new_netif_glue(ethHandles[0])));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                               &ethEventHandler, NULL));

    // Start Ethernet driver state machine
    ESP_ERROR_CHECK(esp_eth_start(ethHandles[0]));
}
