#ifndef __APP_SPIFFS_H__
#define __APP_SPIFFS_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define MAX_SIZE_SCENARIO (2048)
#define LOG_TAG_SPIFFS "SPIFFS"
#define CHUNK_SIZE (1024)

#define PATH_SYSTEM "/RCU"
#define APP_DEVICE_SYSTEM_FILENAME "/RCU/systemConfig.txt"
#define APP_DEVICE_MQTT_INOUT_FILENAME "/RCU/mqttInOut.txt"
#define APP_DEVICE_SCENARIO_LOCAL_FILENAME "/RCU/localScenario.txt"
#define MQTT_MAXIMUM_FILE_INOUT (1024)
#define MAX_SIZE_STRING_PARSE (1024)
#define MAX_SIZE_STRING_RETURN (512)
#define MAX_SIZE_READ_STATIC_IP (1024)
#define MAX_SIZE_READ_MQTT_CONFIG (1024)


#ifdef DEBUG
#define LREP(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define LREP_WARNING(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#define LREP_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define LREP_RAW(format, ...) printf(format, ##__VA_ARGS__)
#else
#define LREP(tag, format, ...) \
    {                          \
    }
#define LREP_WARNING(tag, format, ...) \
    {                                  \
    }
#define LREP_ERROR(tag, format, ...) \
    {                                \
    }
#define LREP_RAW(format, ...) \
    {                         \
    }
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t appSpiffsInit();
    esp_err_t appSpiffsReadFile(const char *fileName, char *buffer,
                                   int maximumLen);
    esp_err_t appSpiffsWriteFile(const char *fileName, const char *buffer);

    bool appSpiffsCheckFileExist(const char *);

    void appSpiffsReadIpConfig(char *ipStart, char *gwStart, char *subStart);

    void appSpiffsReadDeviceBirth();

    void appSpiffsRuleOfflineInit();

    bool getStaticIp(uint32_t *ip, uint32_t *gw, uint32_t *sub);

    bool getMqttConfig(char *broker, int *port, char *username);

    bool getHttpOtaConfig();

#ifdef __cplusplus
}
#endif

#endif /*__APP_SPIFFS_H*/
