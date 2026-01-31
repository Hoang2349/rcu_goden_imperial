#ifndef __HTTPS_OTA_RCU_
#define __HTTPS_OTA_RCU_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define TIME_PERIOD_CHECK_OTA       (60000 / portTICK_PERIOD_MS)

#define MAX_HTTP_OUTPUT_BUFFER      (2048)
#define HOST_SERVER_OTA             "18.140.61.221"
#define PORT_SERVER_OTA             (80)
#define PATH_SERVER_START           "/api/v1/"
#define PATH_ATTRIBUTES_END         "/attributes"
#define PATH_FIRMWARE_END           "/firmware"
#define QUERY_TILTLE_START           "title="
#define QUERY_VERSION_START          "&version="
#define QUERY_SIZE_START             "&size="
#define QUERY_CHUNK_START            "&chunk="

#define DEFAULT_TITLE_STM32         "STM32" //set default
#define DEFAULT_TITLE_ESP32         "ESP32" //SET DEFALT
#define DEFAULT_TITLE_GODEN_IMPERIAL  "GODEN_IMPERIAL" //SET DEFALT

#define CHUNK_SIZE_DOWNLOAD_STM32   (1024)

#define MAX_LENGTH_ACCESS_TOKEN     (25)
#define MAX_LENGTH_HOST      (256)
#define MAX_LENGTH_PATH      (256)
#define MAX_LENGTH_QUERY     (256)
#define CRC32_POLYNOMIAL     (0xEDB88320)

#define TIME_RESET_FLAG       (1000 / portTICK_PERIOD_MS)
#define TIME_WAIT_HTTP_REQ (100 / portTICK_RATE_MS)
#define MAX_LENGTH_NAME_VERSION (10)
#define MAX_LENGTH_STRING_SIZE  (20)
#define SIZE_ADDRESS (4)
#define NUMBER_CHECK_ERROR_STM (3)
typedef struct 
{
    char host[MAX_LENGTH_HOST];
    int port;
    char path[MAX_LENGTH_PATH];
    char query[MAX_LENGTH_QUERY];
}httpsConfig_t;


#ifdef __cplusplus
extern "C"
{
#endif
    void appHttpsOtaInit();
    void httpsOtaStm32();
    void httpsOtaEsp32();
    bool httpsDownloadBinStm32(int size, const char *version,
                               uint32_t crcThingsBoard);
    bool getBinDataStm32onServer(const char *querySet, char *dataOta);
    bool httpsGetAttributesStm32(char *version, int *fwSize, uint32_t *fwCheckSum);
    bool compareVersion(char *newVersion, char *oldVersion);
    void packeQueryOtaStm32(char *querySet, const char *title,
                            const char *version, const char *sizeLoad);
    uint32_t crc32(const char *data, size_t length, uint32_t initial_crc);
    void reverseString(char *str);
    httpsConfig_t parseConfigHttpsGetAttributes();
    void getTokenStm32(const char *tokenEsp32, char *tokenStm32);
    void hanldeStatusStm32();
    void checkHeap();
#ifdef __cplusplus
}
#endif

extern char *versionEsp32;
extern char accessTokenEsp32[MAX_LENGTH_ACCESS_TOKEN];
extern char accessTokenStm32[MAX_LENGTH_ACCESS_TOKEN];

extern char host_ota[MAX_LENGTH_HOST];
extern char title_ota[MAX_LENGTH_STRING_SIZE];
extern uint16_t port_ota;
extern char access_token_ota[MAX_LENGTH_ACCESS_TOKEN];


#endif