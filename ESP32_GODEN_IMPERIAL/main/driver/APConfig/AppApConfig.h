#ifndef __APP_AP_CONFIG_H
#define __APP_AP_CONFIG_H

#include <string.h>

#define ESP_WIFI_SSID "RCU-12345678"
#define ESP_WIFI_PASS "11223344"
#define SIZE_MAC_ADDRESS (6)
#define LENGTH_MAC_ADDRESS (18)
#define ESP_WIFI_CHANNEL (11)
#define MAX_STA_CONN (1)
#define MAX_LEN_NAME_AP (16)
#define TIME_OFF_WIFI_AP (5000 / portTICK_PERIOD_MS)
#define TIME_COUNT (8)
// time_stream = timer_count * Time_off_wifi_AP ex: 5000(ms) * 15 = 75(s)
#define TIME_STA_STREAM (30)

void appWifiApInit();
void wifiApDeinit();

#endif