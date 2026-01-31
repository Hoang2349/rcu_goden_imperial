/*
 * app_spiffs.c
 *
 *  Created on: Jul 13, 2022
 *      Author: trungnc
 */

#include "AppSpiffs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "AppEthernet.h"
#include "Mqtt.h"
#include "MqttDeviceBirth.h"
#include "ParseScen.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "httpsOtaRcu.h"
#include "nvs_flash.h"
#define DEBUG_LOG_PARSE_STRING (0)

#define APP_FS_WRITE_LINE(__BUF, __STR, __FILE)                 \
    do                                                          \
    {                                                           \
        memset(__BUF, 0, sizeof(__BUF));                        \
        sprintf(__BUF, "%s\r\n", (__STR != NULL) ? __STR : ""); \
        fputs(__BUF, __FILE);                                   \
    } while (0)

/// @brief init file spiffs
/// @return
esp_err_t appSpiffsInit()
{
    ESP_LOGI(__FUNCTION__, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = PATH_SYSTEM,
        .partition_label = NULL,
        .max_files = 5,  // This sets the maximum number of files that can be
                         // open at the same time
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(__FUNCTION__, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(__FUNCTION__, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(__FUNCTION__, "Failed to initialize SPIFFS (%s)",
                     esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0, space = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    space = total - used;
    if (ret != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,
                 "Failed to get SPIFFS partition information (%s)",
                 esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGW(__FUNCTION__, "Partition size: total: %d, used: %d, space: %d",
             total, used, space);
    return ESP_OK;
}

/// @brief check status file need to call
/// @param fileName : name file need check
/// @return true or false
bool appSpiffsCheckFileExist(const char *fileName)
{
    struct stat st;
    if (stat(fileName, &st) == 0)
    {
        return true;
    }
    return false;
}

/// @brief Function read datac in file
/// @param fileName : Name file need read
/// @param buffer : point store data before read
/// @param maximumLen : length data read
/// @return ESP_OK or ESP_FAIL
esp_err_t appSpiffsReadFile(const char *fileName, char *buffer, int maximumLen)
{
    FILE *fptr = fopen(fileName, "r");
    if (fptr == NULL)
    {
        LREP_ERROR(__FUNCTION__, "Failed to open file for reading");
        return ESP_FAIL;
    }
    int index = 0;
    int c = fgetc(fptr);
    while (c != EOF)
    {
        buffer[index] = (char)c;
        c = fgetc(fptr);
        if (index++ >= maximumLen) break;
    }
    buffer[index] = '\0';
    fclose(fptr);
    return ESP_OK;
}

/// @brief Write data to file
/// @param fileName : Name file write
/// @param buffer : point to data need save
/// @return ESP_OK or ESP_FAIL
esp_err_t appSpiffsWriteFile(const char *fileName, const char *buffer)
{
    FILE *fPtr = fopen(fileName, "w");
    if (fPtr == NULL)
    {
        LREP_ERROR(__FUNCTION__, "Failed to open file for writing!");
        return ESP_FAIL;
    }
    fputs(buffer, fPtr);
    fclose(fPtr);
    return ESP_OK;
}

/// @brief parse string to object JSON
/// @param stringInput : string containing the json object to be processed
/// @param stringOutput : Excess processing string returned
void packJsonFromStringMqtt(const char *stringInput, char *stringOutput)
{
    char *string = strdup(stringInput);
    char checkStart = '{';
    char checkEnd = '}';
    int countCheck = 0;
    int location = 0;
    char *start = NULL;
    char *end = NULL;
    unsigned int sizeObject;
    int i;

ReParse:
    for (i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == checkStart)
        {
            if (countCheck == 0)
            {
                start = string + i;
                location = i;
            }
            countCheck++;
#if DEBUG_LOG_PARSE_STRING
            printf("count up: %d\n", countCheck);
#endif
        }
        if (string[i] == checkEnd)
        {
            countCheck--;
#if DEBUG_LOG_PARSE_STRING
            printf("count down : %d\n", countCheck);
#endif
            if (countCheck == 0)
            {
                end = string + i + 1;

#if DEBUG_LOG_PARSE_STRING
                sizeObject = i - location;
                printf("object size: %d\n", sizeObject);
#endif
                //--------- handle Json Object hear
                deviceBirth(start);
                //----------- end handle jsonobject

                strcpy(string, end);
                goto ReParse;
            }
        }
    }
    if (countCheck != 0)
    {
        memset(stringOutput, 0, 512);
        strcpy(stringOutput, start);
    }

    free(string);
}

/// @brief Read file contain data debirth MQTT
void appSpiffsReadDeviceBirth()
{
    if (!appSpiffsCheckFileExist(APP_DEVICE_MQTT_INOUT_FILENAME))
    {
        ESP_LOGE(__FUNCTION__, "DEVICE_MQTT_INOUT info file does not exist");
        return;
    }
    FILE *file = fopen(APP_DEVICE_MQTT_INOUT_FILENAME, "r");
    if (file == NULL)
    {
        ESP_LOGE("FILE", "Failed to open file");
        return;
    }
    char bufferIn[MAX_SIZE_STRING_PARSE + 1];
    char bufferOut[MAX_SIZE_STRING_RETURN];
    int size;
    memset(bufferIn, 0, MAX_SIZE_STRING_PARSE);
    memset(bufferOut, 0, MAX_SIZE_STRING_RETURN);
    while (1)
    {
        int sizeReturn = strlen(bufferOut);
        size = fread((bufferIn + sizeReturn), sizeof(char),
                     MAX_SIZE_STRING_PARSE - sizeReturn + 1, file);
        bufferIn[MAX_SIZE_STRING_PARSE + 1] = '\0';
#if DEBUG_LOG_PARSE_STRING
        printf("%s\n", bufferIn);
#endif
        memset(bufferOut, 0, MAX_SIZE_STRING_RETURN);
        /*   User handle string   */
        packJsonFromStringMqtt(bufferIn, bufferOut);
        // end handle string
#if DEBUG_LOG_PARSE_STRING
        printf("\n-----------string data return -----------\n");
        printf("%s", bufferOut);
        printf("\n-----------string end data return -----------\n");
#endif
        memset(bufferIn, 0, MAX_SIZE_STRING_PARSE);
        strcpy(bufferIn, bufferOut);

        if (size < MAX_SIZE_STRING_PARSE - sizeReturn)
        {
#if DEBUG_LOG_PARSE_STRING
            printf("File empty !\n");
#endif
            break;
        }
    }
    fclose(file);
}

/// @brief Parse string to Josn object
/// @param stringInput : string containing the json object to be processed
/// @param stringOutput : Excess processing string returned
void packJsonFromStringOffline(const char *stringInput, char *stringOutput)
{
    char *string = strdup(stringInput);
    char checkStart = '{';
    char checkEnd = '}';
    int countCheck = 0;
    int location = 0;
    char *start = NULL;
    char *end = NULL;
    unsigned int sizeObject;
    int i;

ReParse:
    for (i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == checkStart)
        {
            if (countCheck == 0)
            {
                start = string + i;
                location = i;
            }
            countCheck++;
#if DEBUG_LOG_PARSE_STRING
            printf("count up: %d\n", countCheck);
#endif
        }
        if (string[i] == checkEnd)
        {
            countCheck--;
#if DEBUG_LOG_PARSE_STRING
            printf("count down : %d\n", countCheck);
#endif
            if (countCheck == 0)
            {
                sizeObject = i - location;
                end = string + i + 1;

#if DEBUG_LOG_PARSE_STRING
                printf("object size: %d\n", sizeObject);
#endif
                //--------- handle Json Object here-------------//

                parseDataJsonScenLocal(start);
                //----------- end handle jsonobject-------------//
                strcpy(string, end);
                goto ReParse;
            }
        }
    }
    if (countCheck != 0)
    {
        memset(stringOutput, 0, MAX_SIZE_STRING_RETURN);
        strcpy(stringOutput, start);
    }
    free(string);
}

/// @brief FUNCTION init data rule offline
void appSpiffsRuleOfflineInit()
{
    if (!appSpiffsCheckFileExist(APP_DEVICE_SCENARIO_LOCAL_FILENAME))
    {
        ESP_LOGE(__FUNCTION__, "Rule Local info file does not exist");
        return;
    }
    FILE *file = fopen(APP_DEVICE_SCENARIO_LOCAL_FILENAME, "r");
    if (file == NULL)
    {
        ESP_LOGE("FILE", "Failed to open file");
        return;
    }
    char bufferIn[MAX_SIZE_STRING_PARSE + 1];
    char bufferOut[MAX_SIZE_STRING_RETURN];
    int size;
    memset(bufferIn, 0, MAX_SIZE_STRING_PARSE);
    memset(bufferOut, 0, MAX_SIZE_STRING_RETURN);
    while (1)
    {
        int sizeReturn = strlen(bufferOut);
        size = fread((bufferIn + sizeReturn), sizeof(char),
                     MAX_SIZE_STRING_PARSE - sizeReturn + 1, file);
        // printf("size: %d\n", size);
        bufferIn[MAX_SIZE_STRING_PARSE + 1] = '\0';
#if DEBUG_LOG_PARSE_STRING
        printf("%s\n", bufferIn);
#endif
        memset(bufferOut, 0, MAX_SIZE_STRING_RETURN);
        // User handle string
        packJsonFromStringOffline(bufferIn, bufferOut);
        // end handle string
#if DEBUG_LOG_PARSE_STRING
        printf("\n-----------string data return -----------\n");
        printf("%s", bufferOut);
        printf("\n-----------string end data return -----------\n");
#endif
        memset(bufferIn, 0, MAX_SIZE_STRING_PARSE);
        strcpy(bufferIn, bufferOut);
        if (size < MAX_SIZE_STRING_PARSE - sizeReturn)
        {
#if DEBUG_LOG_PARSE_STRING
            printf("File empty !\n");
#endif
            break;
        }
    }
    fclose(file);
}

/// @brief get static ip
/// @return true or false
bool getStaticIp(uint32_t *ip, uint32_t *gw, uint32_t *sub)
{
    if (!appSpiffsCheckFileExist(APP_DEVICE_SYSTEM_FILENAME))
    {
        ESP_LOGE(__FUNCTION__, "System info file does not exist");
        return false;
    }
    char *string = (char *)malloc(MAX_SIZE_READ_STATIC_IP * sizeof(char));
    appSpiffsReadFile(APP_DEVICE_SYSTEM_FILENAME, string,
                      MAX_SIZE_READ_STATIC_IP);
#if DEBUG_LOG_PARSE_STRING
    printf("sting read: %s\n", string);
#endif
    cJSON *root = cJSON_Parse(string);
    if (root == NULL)
    {
        ESP_LOGE(__FUNCTION__, "File config erorr!!!");
        free(string);
        return false;
    }
    cJSON *ipConfig = cJSON_GetObjectItem(root, "ipConfig");
    cJSON *ipSet = cJSON_GetObjectItem(ipConfig, "ip");
    cJSON *gwSet = cJSON_GetObjectItem(ipConfig, "gateway");
    cJSON *subSet = cJSON_GetObjectItem(ipConfig, "netmask");
    if (ipConfig == NULL || ipSet == NULL || gwSet == NULL || subSet == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Parametter  erorr!!!");
        free(string);
        cJSON_Delete(root);
        return false;
    }
    *ip = esp_ip4addr_aton(ipSet->valuestring);
    *gw = esp_ip4addr_aton(gwSet->valuestring);
    *sub = esp_ip4addr_aton(subSet->valuestring);
    free(string);
    cJSON_Delete(root);
    return true;
}

/// @brief get config mqtt client
/// @param broker : host set mqtt
/// @param port : port open mqtt
/// @param username : AccesToken device
/// @return true or false
bool getMqttConfig(char *broker, int *port, char *username)
{
    if (!appSpiffsCheckFileExist(APP_DEVICE_SYSTEM_FILENAME))
    {
        ESP_LOGE(__FUNCTION__, "System info file does not exist");
        return false;
    }
    char *string = (char *)malloc(MAX_SIZE_READ_MQTT_CONFIG * sizeof(char));
    appSpiffsReadFile(APP_DEVICE_SYSTEM_FILENAME, string,
                      MAX_SIZE_READ_MQTT_CONFIG);
#if DEBUG_LOG_PARSE_STRING
    printf("sting read: %s\n", string);
#endif
    cJSON *root = cJSON_Parse(string);
    if (root == NULL)
    {
        ESP_LOGE(__FUNCTION__, "File config erorr!!!");
        free(string);
        return false;
    }
    cJSON *mqttConfig = cJSON_GetObjectItem(root, "mqttConfig");
    cJSON *brokerSet = cJSON_GetObjectItem(mqttConfig, "broker");
    cJSON *portSet = cJSON_GetObjectItem(mqttConfig, "port");
    cJSON *usernameSet = cJSON_GetObjectItem(mqttConfig, "username");
    if (mqttConfig == NULL || brokerSet == NULL || portSet == NULL ||
        usernameSet == NULL)
    {
        free(string);
        cJSON_Delete(root);
        return false;
    }
    strcpy(broker, brokerSet->valuestring);
    *port = portSet->valueint;
    strcpy(username, usernameSet->valuestring);
    free(string);
    cJSON_Delete(root);
    return true;
}

bool getHttpOtaConfig()
{
    if (!appSpiffsCheckFileExist(APP_DEVICE_SYSTEM_FILENAME))
    {
        ESP_LOGE(__FUNCTION__, "System info file does not exist");
        return false;
    }
    char *string = (char *)malloc(MAX_SIZE_READ_MQTT_CONFIG * sizeof(char));
    appSpiffsReadFile(APP_DEVICE_SYSTEM_FILENAME, string,
                      MAX_SIZE_READ_MQTT_CONFIG);
    cJSON *root = cJSON_Parse(string);
    if (root == NULL)
    {
        ESP_LOGE(__FUNCTION__, "File config erorr!!!");
        free(string);
        return false;
    }

    cJSON *httpOtaConfig = cJSON_GetObjectItem(root, "httpOtaConfig");
    cJSON *host = cJSON_GetObjectItem(httpOtaConfig, "host");
    cJSON *port = cJSON_GetObjectItem(httpOtaConfig, "port");
    cJSON *title = cJSON_GetObjectItem(httpOtaConfig, "title");
    cJSON *acces_ota = cJSON_GetObjectItem(httpOtaConfig, "username");

    if (httpOtaConfig == NULL || host == NULL || port == NULL || title == NULL || 
        acces_ota == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Parametter  erorr!!!");
        free(string);
        cJSON_Delete(root);
        return false;
    }
    strcpy(host_ota, host->valuestring);
    strcpy(title_ota, title->valuestring);
    strcpy(access_token_ota, acces_ota->valuestring);
    port_ota = port->valueint;
    // free poiter string
    free(string);
    cJSON_Delete(root);
    return true;
}
