/*  WiFi softAP
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "AppApConfig.h"

#include <string.h>

#include "AppHttpServer.h"
#include "AppSpiffs.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"

TimerHandle_t oneShortTimer;
uint8_t timerTick = TIME_COUNT;

/**
 * @brief Function handle event data
 * @param void *arg Not handle
 * @param esp_event_base_t eventBase Not handle
 * @param int32_t eventId id event, each event corresponds to each ID
 * @param void *eventData Includes event data
 */
static void wifiEventHandle(void *arg, esp_event_base_t eventBase,
                            int32_t eventId, void *eventData)
{
    switch (eventId)
    {
        case WIFI_EVENT_AP_STACONNECTED:
        {
            wifi_event_ap_staconnected_t *event =
                (wifi_event_ap_staconnected_t *)eventData;
            ESP_LOGI(__FUNCTION__, "station " MACSTR " join, AID=%d",
                     MAC2STR(event->mac), event->aid);
            timerTick = TIME_STA_STREAM;
            ESP_ERROR_CHECK(startFileServer(PATH_SYSTEM));
            ESP_LOGI(__FUNCTION__, "File server started");
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *event =
                (wifi_event_ap_stadisconnected_t *)eventData;
            ESP_LOGI(__FUNCTION__, "station " MACSTR " leave, AID=%d",
                     MAC2STR(event->mac), event->aid);
            timerTick = 1;
            stopFileServer();
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Function init Wifi mode Access Point, connect open webserver about 2
 * minute
 */
void appWifiApInit()
{
    esp_netif_create_default_wifi_ap();
    wifi_config_t wifiConfig = {
        .ap = {.ssid = "",
               .ssid_len = MAX_LEN_NAME_AP,
               .channel = ESP_WIFI_CHANNEL,
               .password = ESP_WIFI_PASS,
               .max_connection = MAX_STA_CONN,
               .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint8_t macWifiApMode[SIZE_MAC_ADDRESS] = {0};
    char strMacAddress[LENGTH_MAC_ADDRESS] = {0};
    char userNameWifi[LENGTH_MAC_ADDRESS] = {0};

    esp_wifi_get_mac(WIFI_IF_AP, macWifiApMode);
    ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, macWifiApMode, 6, ESP_LOG_INFO);
    snprintf(strMacAddress, sizeof(strMacAddress), "%02X%02X%02X%02X%02X%02X",
             macWifiApMode[0], macWifiApMode[1], macWifiApMode[2],
             macWifiApMode[3], macWifiApMode[4], macWifiApMode[5]);

    strcpy(userNameWifi, "RCU-");
    strcat(userNameWifi, strMacAddress);
    for (int i = 0; i < LENGTH_MAC_ADDRESS - 2; i++)
    {
        wifiConfig.ap.ssid[i] = userNameWifi[i];
    }
    ESP_LOGW(__FUNCTION__, "SSID AP: %s\n", wifiConfig.ap.ssid);
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandle, NULL, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(__FUNCTION__,
             "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             wifiConfig.ap.ssid, wifiConfig.ap.password, ESP_WIFI_CHANNEL);
    printf("Mac ID Wifi AP: %s\n", wifiConfig.ap.ssid);
    // ---------------init timer one short for ap --------------------//
    oneShortTimer = xTimerCreate("Handle AP on_of", TIME_OFF_WIFI_AP, pdTRUE, 0,
                                 (TimerCallbackFunction_t)wifiApDeinit);
    if (oneShortTimer == NULL)
    {
        esp_restart();
    }
    ESP_LOGI(__FUNCTION__, "Create timer succes");
    if (xTimerStart(oneShortTimer, 0) != pdPASS)
    {
        ESP_LOGI(__FUNCTION__,
                 "The timer could not be set into the Active state");
    }
}

/**
 * @brief Function Deinit Wifi Ap mode, free all data and delete timmer
 */
void wifiApDeinit()
{
    // printf("Count ----timer: %d\n", timerTick);
    if (timerTick == 0)
    {
        esp_wifi_stop();
        esp_wifi_clear_ap_list();
        esp_wifi_deinit();
        xTimerDelete(oneShortTimer, 1);
        ESP_LOGW(__FUNCTION__, "Deinit Wifi AP!");
    }
    else
        timerTick--;
}
