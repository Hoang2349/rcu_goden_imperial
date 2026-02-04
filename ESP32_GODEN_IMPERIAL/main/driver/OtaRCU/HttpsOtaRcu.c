#include "HttpsOtaRcu.h"

#include <stdbool.h>
#include <string.h>

#include "AppSpiffs.h"
#include "Modbus.h"
#include "cJSON.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_tls.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "otaStm32.h"
#include "app_goden_imperial_common.h"  
#include "Mqtt.h"

//=================define version for esp32==================
// version release project GODEN IMPERIAL RCU
char *versionEsp32 = "1.2.4";
//=================end define version for esp32==================

char versionStm32[MAX_LENGTH_NAME_VERSION] = {0};
char accessTokenEsp32[MAX_LENGTH_ACCESS_TOKEN] = {0};
char accessTokenStm32[MAX_LENGTH_ACCESS_TOKEN] = {0};

char host_ota[MAX_LENGTH_HOST] = HOST_SERVER_OTA;
char title_ota[MAX_LENGTH_STRING_SIZE] = DEFAULT_TITLE_GODEN_IMPERIAL;
uint16_t port_ota = PORT_SERVER_OTA;
char access_token_ota[MAX_LENGTH_ACCESS_TOKEN] = {0};


/// @brief  http callback when have event return
/// @param evt ponter include data event return
/// @return status OK or FAIL
esp_err_t httpEventHandle(esp_http_client_event_t *evt)
{
    static char *outputBuffer;  // Buffer to store response of http request
                                // from event handler
    static int outputLen;       // Stores number of bytes read
    switch (evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_HEADER, key=%s, value=%s",
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client))
            {
                if (evt->user_data)
                {
                    memcpy(evt->user_data + outputLen, evt->data,
                           evt->data_len);
                }
                else
                {
                    if (outputBuffer == NULL)
                    {
                        // We initialize outputBuffer with 0 because it is used
                        // by strlen() and similar functions therefore should be
                        // null terminated.
                        uint16_t lent = esp_http_client_get_content_length(evt->client);
                        outputBuffer = (char *)malloc(lent + 1);
                        outputLen = 0;
                        if (outputBuffer == NULL)
                        {
                            ESP_LOGE(
                                __FUNCTION__,
                                "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(outputBuffer + outputLen, evt->data, evt->data_len);
                }
                outputLen += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_FINISH");
            if (outputBuffer != NULL)
            {
                free(outputBuffer);
                outputBuffer = NULL;
            }
            outputLen = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(__FUNCTION__, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err =
                esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0)
            {
                ESP_LOGI(__FUNCTION__, "Last esp error code: 0x%x", err);
                ESP_LOGI(__FUNCTION__, "Last mbedtls failure: 0x%x",
                         mbedtls_err);
            }
            if (outputBuffer != NULL)
            {
                free(outputBuffer);
                outputBuffer = NULL;
            }
            outputLen = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(__FUNCTION__, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

/// @brief Fuction main task check update for esp and stm32
/// @param pvParameters : no use
static void httpsCheckVersionOTA(void *pvParameters)
{
    bool ret = getHttpOtaConfig();
    while (1)
    {
        vTaskDelay(TIME_PERIOD_CHECK_OTA);
        if(pms_room_status.status_room == ROOM_RENTED)
        {
            ESP_LOGW(__FUNCTION__, "Room is Rented, skip OTA check");
            continue;
        }
        httpsOtaEsp32();

        //TODO: update status firmware for esp32 
    }
}

/// @brief creat task hanlde OTA RCU
void appHttpsOtaInit()
{
    xTaskCreate(&httpsCheckVersionOTA, "Check new version", 1024 * 8, NULL, 7,
                NULL);
}

/// @brief Function check version and process OTA STM32
void httpsOtaStm32()
{
    ESP_LOGW(__FUNCTION__, "//==========httpsOtaStm32=========//");
    char newVersion[MAX_LENGTH_NAME_VERSION] = {0};
    uint32_t fwCheckSum = 0;
    int fwSize = 0;
    if (strcmp(accessTokenStm32, "") == 0)
    {
        getTokenStm32(accessTokenEsp32, accessTokenStm32);
        ESP_LOGW(__FUNCTION__, "Get AccessToken Success: \"%s\"",
                 accessTokenStm32);
    }
    if (!httpsGetAttributesStm32(newVersion, &fwSize, &fwCheckSum))
    {
        ESP_LOGE(__FUNCTION__, "HTTPs get Version false");
        return;
    }
    ESP_LOGI(__FUNCTION__, "%s", "1.Get Version via https success");
    if (!flagModbusEmpty)
    {
        return;
    }
    if (!strlen(versionStm32))
    {
        ESP_LOGE(__FUNCTION__, "Get version STM32 false");
        // retry get version
        getVersionStm32(versionStm32);
        return;
    }
    ESP_LOGI(__FUNCTION__, "2.Get old versin STM32: %s", versionStm32);
    // compare
    if (!compareVersion(newVersion, versionStm32))
    {
        ESP_LOGW(__FUNCTION__, "STM32 No Update");
        return;
    }
    ESP_LOGI(__FUNCTION__, "%s", "3.Have new change");
    if (httpsDownloadBinStm32(fwSize, newVersion, fwCheckSum))
    {
        strcpy(versionStm32, newVersion);
        ESP_LOGW(__FUNCTION__, "============================");
        ESP_LOGI(__FUNCTION__, "STM32 OTA newversion Sucess");
        ESP_LOGW(__FUNCTION__, "============================");
        // todo success thingsboard
    }
    ESP_LOGW(__FUNCTION__, " STM32 No Update");
}

/*
    thuc hien down load file .bin cho STM32 tren thingsboard
    @ int size                : kích thước file .bin download
    @ const char *version     : version download
    @ uint32_t crcThingsBoard : code crc get in thingsboard
    @ return                  : OTA success or false
*/
bool httpsDownloadBinStm32(int size, const char *version,
                           uint32_t crcThingsBoard)
{
    ESP_LOGI(__FUNCTION__, "----------------Write Flash Stm32---------------");

    // access Boot mode STm32
    if (!cmdStartBootStm32())
    {
        ESP_LOGE(__FUNCTION__, "Call Boot Erorr");
    }
    ESP_LOGI(__FUNCTION__, "1.mode boot");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Vào che do xoa bo nhơ và xoa bo nho vung Applycation
    if (!cmdEraseFlashStm32())
    {
        ESP_LOGE(__FUNCTION__, "CMD Erase Erorr");
        if (!cmdExitBootStm32())
        {
            ESP_LOGE(__FUNCTION__, "Cannot exit boot");
        }
        return false;
    }
    ESP_LOGI(__FUNCTION__, "2.vao mode xoa thanh cong");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // thuc hien xoa bo nho Applycation
    if (!eraseFlashMemoryAppStm32())
    {
        ESP_LOGE(__FUNCTION__, "Erase Application False");
        if (!cmdExitBootStm32())
        {
            ESP_LOGE(__FUNCTION__, "Cannot exit boot");
        }
        return false;
    }
    ESP_LOGI(__FUNCTION__, "3.xoa thanh cong");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // start write flash stm32
    char loadAddress[SIZE_ADDRESS] = {0x08, 0x00, 0x68, 0x00};
    char querySetTemp[MAX_LENGTH_QUERY] = {0};
    int count = 0;
    char stringSize[MAX_LENGTH_STRING_SIZE] = {0};
    uint32_t crcCheck = 0xFFFFFFFF;
    char dataFlash[CHUNK_SIZE_DOWNLOAD_STM32] = {0xFF};
    int sizeOtaRunning = 0;
    sprintf(stringSize, "%d", CHUNK_SIZE_DOWNLOAD_STM32);
    packeQueryOtaStm32(querySetTemp, DEFAULT_TITLE_STM32, version,
                       &stringSize[0]);
    do
    {
        ESP_LOGI(__FUNCTION__, "Flasg OTA Page: %d", count);
        char querySet[MAX_LENGTH_QUERY] = {0};
        char chunkIndex[MAX_LENGTH_STRING_SIZE] = {0};
        memset(dataFlash, 0xFF, CHUNK_SIZE_DOWNLOAD_STM32);
        strcpy(querySet, querySetTemp);
        sprintf(chunkIndex, "%d", count);
        strcat(querySet, chunkIndex);
        int len = strlen(querySet);
        count++;
        if (!getBinDataStm32onServer((const char *)querySet, dataFlash))
        {
            ESP_LOGE(__FUNCTION__, "Get file .bin Erorr!");
            return false;
        }

        // OTA STM32
        if (!writeFlashStm32(loadAddress, dataFlash, CHUNK_SIZE_DOWNLOAD_STM32))
        {
            ESP_LOGE(__FUNCTION__, "Write flash false");
            return false;
        }
        sizeOtaRunning = count * CHUNK_SIZE_DOWNLOAD_STM32;
        // caculator CRC32
        if (sizeOtaRunning < size)
        {
            crcCheck = crc32(dataFlash, CHUNK_SIZE_DOWNLOAD_STM32, crcCheck);
        }
        else
        {
            int lenCrc = CHUNK_SIZE_DOWNLOAD_STM32 - (sizeOtaRunning - size);
            crcCheck = crc32(dataFlash, lenCrc, crcCheck);
        }
    } while (sizeOtaRunning < size);

    // to do check CRC with thingsBoard
    if (~crcCheck != crcThingsBoard)
    {
        ESP_LOGE(__FUNCTION__, "Loi goi firmware ThingsBoard");
        return false;
    }
    // to do check CRC with STM32
    uint32_t crc32Respond = 0;
    if (!getCrcStm32Flash(&crc32Respond))
    {
        ESP_LOGE(__FUNCTION__, "Loi goi firmware CRC STm32");
        // todo exit boot STM32
        return false;
    }
    // compare crc32
    if (~crcCheck != crc32Respond)
    {
        ESP_LOGE(__FUNCTION__, "Write Flash false CRC Erorr");
        return false;
    }
    // write version for STM32
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (!cmdWriteVersionStm32())
    {
        ESP_LOGE(__FUNCTION__, "Vao che do ghi version loi");
        return false;
    }
    if (!writeNewVersionStm32(version))
    {
        ESP_LOGE(__FUNCTION__, "Ghi version loi");
        return false;
    }
    // exit boot mode
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(__FUNCTION__, "%s", "Ending Connection");
    if (!cmdExitBootStm32())
    {
        ESP_LOGE(__FUNCTION__, "Cannot exit boot");
    }
    ESP_LOGI(__FUNCTION__, "%s", "End OTA STM32");
    return true;
}

/*
    Thuc hien downLoad tung chunk data on server
    @ const char *querySet : query config
    @ char *dataOta        : data server tra ve
    @ return               : true or false
*/
bool getBinDataStm32onServer(const char *querySet, char *dataOta)
{
    // char buf[CHUNK_SIZE_DOWNLOAD_STM32] = {0};
    char pathSet[MAX_LENGTH_PATH] = {0};
    strcpy(pathSet, PATH_SERVER_START);
    strcat(pathSet, accessTokenStm32);
    strcat(pathSet, PATH_FIRMWARE_END);
    esp_http_client_config_t config = {
        .host = HOST_SERVER_OTA,
        .port = PORT_SERVER_OTA,
        .path = pathSet,
        .query = querySet,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = httpEventHandle,
        .method = HTTP_METHOD_GET,
        // .cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,
        .user_data = dataOta,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    vTaskDelay(TIME_WAIT_HTTP_REQ);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Error perform http request %s",
                 esp_err_to_name(err));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }
    if (esp_http_client_get_status_code(client) != 200)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return true;
}

/*
    Ham thuc hien lay cac thong tin version, size file bin, checkSum tren
    thingsboard
    @ char *version         : lay version tren thingsboard
    @ int *fwSize           : kisch thuoc file bin
    @ uint32_t *fwCheckSum  : crc32 cua file .bin
    @ return                : true or false
*/
bool httpsGetAttributesStm32(char *version, int *fwSize, uint32_t *fwCheckSum)
{
    char localRespondBuffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    httpsConfig_t userConfig = parseConfigHttpsGetAttributes(accessTokenStm32);
    esp_http_client_config_t config = {
        .host = userConfig.host,
        .port = userConfig.port,
        .path = userConfig.path,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = httpEventHandle,
        .method = HTTP_METHOD_GET,
        // .cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,
        .user_data = localRespondBuffer,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    vTaskDelay(TIME_WAIT_HTTP_REQ);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Error perform http request %s",
                 esp_err_to_name(err));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }
    if (esp_http_client_get_status_code(client) != 200)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    cJSON *root = cJSON_Parse(localRespondBuffer);
    if (root == NULL)
    {
        // Failed to parse JSON
        ESP_LOGE(__FUNCTION__, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return false;
    }

    cJSON *shared = cJSON_GetObjectItem(root, "shared");
    if (shared != NULL)
    {
        cJSON *fw_checksum = cJSON_GetObjectItem(shared, "fw_checksum");
        cJSON *fw_size = cJSON_GetObjectItem(shared, "fw_size");
        cJSON *fw_tag = cJSON_GetObjectItem(shared, "fw_tag");
        cJSON *fw_version = cJSON_GetObjectItem(shared, "fw_version");
        // Add more items as needed

        if (fw_checksum != NULL && fw_size != NULL && fw_version != NULL)
        {
            // parse version
            strcpy(version, fw_version->valuestring);
            // parse size file .bim
            *fwSize = fw_size->valueint;
            // parse crc32 file .bin
            char crc[MAX_LENGTH_STRING_SIZE] = {0};
            strcpy(crc, fw_checksum->valuestring);
            reverseString(crc);
            int i = 0;
            for (i = 0; i < 8; i++)
            {
                char a = crc[i];
                char b = crc[i + 1];
                crc[i] = b;
                crc[i + 1] = a;
                i = i + 1;
            }
            *fwCheckSum = strtoul(crc, NULL, 16);
            // cJSON_Delete(shared);
            cJSON_Delete(root);
            return true;
        }
    }
    cJSON_Delete(root);
    return false;
}

/*
    Ham thuc hien OTA cho ESP32
*/
void httpsOtaEsp32()
{
    ESP_LOGW(__FUNCTION__, "//==========httpsOtaEsp32=========//");
    char newVersion[MAX_LENGTH_NAME_VERSION] = {0};

    // get version on https
    char localRespondBuffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    httpsConfig_t userConfig = parseConfigHttpsGetAttributes(accessTokenEsp32);
    esp_http_client_config_t config = {
        .host = userConfig.host,
        .port = userConfig.port,
        .path = userConfig.path,
        .event_handler = httpEventHandle,
        .method = HTTP_METHOD_GET,
        .user_data = localRespondBuffer,
    };

    // debug config https
    ESP_LOGI(__FUNCTION__, "Host: %s", config.host);
    ESP_LOGI(__FUNCTION__, "Port: %d", config.port);
    ESP_LOGI(__FUNCTION__, "Path: %s", config.path);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    vTaskDelay(TIME_WAIT_HTTP_REQ);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Error perform http request %s",
                 esp_err_to_name(err));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }
    if (esp_http_client_get_status_code(client) != 200)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    ESP_LOGI(__FUNCTION__, "Get version on https success");
    ESP_LOGI(__FUNCTION__, "Response: %s", localRespondBuffer);
    cJSON *root = cJSON_Parse(localRespondBuffer);
    if (root == NULL)
    {
        // Failed to parse JSON
        ESP_LOGE(__FUNCTION__, "Error parsing JSON: %s\n",
        cJSON_GetErrorPtr()); return;
    }
    cJSON *shared = cJSON_GetObjectItem(root, "shared");
    if (shared == NULL)
    {
        cJSON_Delete(root);
    }
    cJSON *fw_checksum = cJSON_GetObjectItem(shared, "fw_checksum");
    cJSON *fw_size = cJSON_GetObjectItem(shared, "fw_size");
    cJSON *fw_tag = cJSON_GetObjectItem(shared, "fw_tag");
    cJSON *fw_version = cJSON_GetObjectItem(shared, "fw_version");
    // Add more items as needed

    if (fw_checksum != NULL && fw_size != NULL && fw_tag != NULL)
    {
        strcpy(newVersion, fw_version->valuestring);
        // *fwSize = fw_size->valueint;
    }
    cJSON_Delete(root);
    // compare version
    if (!compareVersion(newVersion, versionEsp32))
    {
        ESP_LOGW(__FUNCTION__, "ESP32 No update");
        return;
    }

    // Config OTA ESP32
    char querySet[MAX_LENGTH_QUERY] = {0};
    char pathSet[MAX_LENGTH_PATH] = {0};

    // parse path config
    strcpy(pathSet, PATH_SERVER_START);
    strcat(pathSet, accessTokenEsp32);
    strcat(pathSet, PATH_FIRMWARE_END);

    // parse query config
    strcpy(querySet, QUERY_TILTLE_START);
    strcat(querySet, title_ota);
    strcat(querySet, QUERY_VERSION_START);
    strcat(querySet, newVersion);
    esp_http_client_config_t configOta = {
        .cert_pem = NULL,
        .skip_cert_common_name_check = true,
        .host = host_ota,
        .port = port_ota,
        .path = pathSet,
        .query = querySet,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t otaEspConfig = {
        .http_config = &configOta,
        .max_http_request_size = MAX_HTTP_OUTPUT_BUFFER,
    };

    // PUSH STATUS OTA ESP32
    publish_data_mqtt("{\"fw_state\": \"UPDATING\"}");
    // start OTA ESP32
    esp_err_t ret = esp_https_ota(&otaEspConfig);
    if (ret == ESP_OK)
    {
        ESP_LOGW(__FUNCTION__, "//=======================================//");
        ESP_LOGW(__FUNCTION__, "OTA Succeed, Rebooting...");
        ESP_LOGW(__FUNCTION__, "//=======================================//");
        esp_restart();
    }
    ESP_LOGE(__FUNCTION__, "Firmware upgrade failed---> %d", ret);
}

/// @brief Calculator version from string
/// @param version string need cal
/// @return  version before calculator
int calculatorVersion(const char *version)
{
    int a = atoi(&version[0]);
    int b = atoi(&version[2]);
    int c = atoi(&version[4]);
    return (a * 100 + b * 10 + c);
}

/*
   Ham thuc hien so sanh version cua
   @char *newVersion    : Version moi lay tren thingsBoard
   @char *oldVersion    : Version hien tai cua thiet bi
   @return              : true neu version khac nhau, flase nguoc lai
*/
bool compareVersion(char *newVersion, char *oldVersion)
{
    int new = calculatorVersion(newVersion);
    int old = calculatorVersion(oldVersion);
    if (new > old) return true;
    return false;
}

/// @brief parse string to query get http
/// @param querySet  pointer output
/// @param title title need set
/// @param version string ver sion need get
/// @param sizeLoad size chunk
void packeQueryOtaStm32(char *querySet, const char *title, const char *version,
                        const char *sizeLoad)
{
    strcat(querySet, QUERY_TILTLE_START);
    strcat(querySet, title);
    strcat(querySet, QUERY_VERSION_START);
    strcat(querySet, version);
    strcat(querySet, QUERY_SIZE_START);
    strcat(querySet, sizeLoad);
    strcat(querySet, QUERY_CHUNK_START);
}

/// @brief Caculator crc firmware crc32
/// @param data data input
/// @param length length of data
/// @param initialCrcStart
/// @return  crc of data
uint32_t crc32(const char *data, size_t length, uint32_t initialCrcStart)
{
    uint32_t crc = initialCrcStart;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint32_t)data[i];

        for (int j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ ((crc & 1) * CRC32_POLYNOMIAL);
        }
    }
    return crc;
}

/// @brief function reverseString
/// @param str string need swap
void reverseString(char *str)
{
    int length = strlen(str);
    int start = 0;
    int end = length - 1;

    while (start < end)
    {
        // Swap characters at start and end positions
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;

        // Move towards the center
        start++;
        end--;
    }
}

/// @brief parse config for RCU get Attributes
/// @param accessToken : accessToken device need get attributes
/// @return httsConfig_t variable
httpsConfig_t parseConfigHttpsGetAttributes()
{
    httpsConfig_t config = {
        .host = HOST_SERVER_OTA,
        .port = port_ota,
    };
    strcpy(config.host, host_ota);
    strcpy(config.path, PATH_SERVER_START);
    strcat(config.path, access_token_ota);
    strcat(config.path, PATH_ATTRIBUTES_END);
    return config;
}

/// @brief  Get AccessToken of stm32 via accestoken esp32
/// @param tokenEsp32 : access token essp32
/// @param tokenStm32 : accesToken stm32 retrun pointer
void getTokenStm32(const char *tokenEsp32, char *tokenStm32)
{
    char localRespondBuffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    httpsConfig_t userConfig = parseConfigHttpsGetAttributes(tokenEsp32);
    esp_http_client_config_t config = {
        .host = userConfig.host,
        .port = userConfig.port,
        .path = userConfig.path,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .event_handler = httpEventHandle,
        .method = HTTP_METHOD_GET,
        // .cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,
        .user_data = localRespondBuffer,
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    vTaskDelay(TIME_WAIT_HTTP_REQ);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "Error perform http request %s",
                 esp_err_to_name(err));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }
    if (esp_http_client_get_status_code(client) != 200)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    cJSON *root = cJSON_Parse(localRespondBuffer);
    if (root == NULL)
    {
        // Failed to parse JSON
        ESP_LOGE(__FUNCTION__, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }
    cJSON *shared = cJSON_GetObjectItem(root, "shared");
    if (shared != NULL)
    {
        cJSON *stm32DeviceAccessToken =
            cJSON_GetObjectItem(shared, "stm32DeviceAccessToken");

        if (stm32DeviceAccessToken != NULL)
        {
            strcpy(tokenStm32, stm32DeviceAccessToken->valuestring);
            printf("Access STM32: %s\n", tokenStm32);
            cJSON_Delete(root);
        }
    }
}

/// @brief Function check status running of STM32
void hanldeStatusStm32()
{
    ESP_LOGW(__FUNCTION__, "//==========hanldeStatusStm32=========//");
    int statusStm32 = 0;
    if (xSemaphoreTake(xMutexStatusStm32, (TickType_t)100) == pdTRUE)
    {
        if (flagStatusStm32 == true)
        {
            ESP_LOGI(__FUNCTION__, "Get status STM32 success");
            xSemaphoreGive(xMutexStatusStm32);
            return;
        }
        xSemaphoreGive(xMutexStatusStm32);
    }
    if (!flagModbusEmpty)
    {
        return;
    }
    if (!getStatusStm32(&statusStm32))
    {
        // goi lai 3 lan neu van false thuc hien OTA
        for (int i = 0; i < NUMBER_CHECK_ERROR_STM; i++)
        {
            if (!flagModbusEmpty)
            {
                i--;
                continue;
            }
            if (!getStatusStm32(&statusStm32))
            {
                ESP_LOGE(__FUNCTION__, "%d. STM32 flase", i);
                vTaskDelay(5000 / portTICK_RATE_MS);
                continue;
            }
            break;
        }
        char newVersion[10] = {0};
        uint32_t fwCheckSum = 0;
        int fwSize = 0;
        // to do check status Stm32
        if (!httpsGetAttributesStm32(newVersion, &fwSize, &fwCheckSum))
        {
            ESP_LOGE(__FUNCTION__, "HTTPs get Version false");
            return;
        }
        if (httpsDownloadBinStm32(fwSize, newVersion, fwCheckSum))
        {
            strcpy(versionStm32, newVersion);
            ESP_LOGW(__FUNCTION__, "============================");
            ESP_LOGI(__FUNCTION__, "STM32 OTA newversion Sucess");
            ESP_LOGW(__FUNCTION__, "============================");
            // todo success thingsboard
        }
    }
    ESP_LOGW(__FUNCTION__, "//==========End hanldeStatusStm32=========//");
}

/// @brief creat timer task 1sec
void timerStatusStm32()
{
    static uint8_t count = 0;
    if (xSemaphoreTake(xMutexStatusStm32, (TickType_t)100) == pdTRUE)
    {
        if (flagStatusStm32 == true)
        {
            count++;
            // 5s set flag
            if (count % 5 == 0)
            {
                count = 0;
                ESP_LOGI(__FUNCTION__, "Timer set flag");
                flagStatusStm32 = false;
            }
        }
        xSemaphoreGive(xMutexStatusStm32);
    }
}
