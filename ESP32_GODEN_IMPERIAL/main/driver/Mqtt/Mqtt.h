#ifndef __MQTT_H__
#define __MQTT_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "freertos/FreeRTOS.h"
#define MQTT_BROKER "mqtt://54.179.183.205"
#define MQTT_PORT (1883)
#define MQTT_USER "ZRGb0s5F1vtjl35Zivxi"
#define MQTT_PASS "12345678"
#define MQTT_KEEPALIVE (30)
#define MQTT_LWT_TOPIC "spBv1.0/Sparkplug Group 1/NDEATH/Edge Node"
#define TIME_POOL_QUEUE_RECIVE (100)
#define TIMEOUT_SEND_DATA_QUEUE (10)

#define MAX_LEN_MQTT_HOST (100)
#define MAX_LEN_MQTT_USER (50)
#define MAX_LEN_MQTT_PASS (50)

#define TOPIC_EXAMPLE_DBIRTH "spBv1.0/Sparkplug Group 1/DBIRTH/Edge Node/"
#define TOPIC_EXAMPLE_DDATA "spBv1.0/Sparkplug Group 1/DDATA/Edge Node/"
#define TOPIC_EXAMPLE_DCMD "spBv1.0/Sparkplug Group 1/DCMD/Edge Node/"
#define TOPIC_EXAMPLE_NDEATH "spBv1.0/Sparkplug Group 1/NDEATH/Edge Node"

// FOR PROJECT GODEN IMPERIAL
#define TOPIC_ATTRIBUTES_TOPIC "v1/devices/me/attributes"
#define TOPIC_RPC_REQUEST_TOPIC "v1/devices/me/rpc/request/+"
#define TOPIC_DEVICE_TELEMETRY "v1/devices/me/telemetry"

#define MQTT_MAXIMUM_TOPIC_LENGTH (128)
#define MQTT_MAXIMUM_MESSAGE_LENGTH (512)
#define MQTT_MAXIMUM_MESSAGE_RECIVE_LENGTH (256)
#define MAX_SIZE_MQTT_RECIVE (100)
#define QOS_SET_LEVEL (1)
#define RETAIN_SET_DISABLE (0)

typedef struct
{
    char mqttHost[MAX_LEN_MQTT_HOST];
    uint16_t mqttPort;
    char mqttUser[MAX_LEN_MQTT_USER];
    char mqttPass[MAX_LEN_MQTT_PASS];
} mqttConfig_t;

typedef struct
{
    char strTopic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char strPayload[MQTT_MAXIMUM_MESSAGE_RECIVE_LENGTH];
    int lenTopic;
    int lenPayload;
    int deviceId;
} sMqttPkg_t;

typedef struct
{
    char key[32];
    char value[64];
    int payloadLen;
} state_in_out_t;
typedef enum {
    TYPE_STRING,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_DOUBLE
} value_type_t;
#ifdef __cplusplus
extern "C"
{
#endif
    void appMqttInit();

    void appMqttPublish(char *topic, char *payload, int payloadLen);
    void appMqttHandleDataRecive(sMqttPkg_t *mqttPkg);
    void getNodeDeathPayload(char *payload, int *len);
    void cpyArray(char *des, char *src, uint16_t pStartReadSrc, uint16_t len);
    void publish_data_mqtt(const char* data);
    // function add for PR Goden Imperial
    bool push_state_inout(const state_in_out_t *item, uint32_t timeout_ticks);
    bool pop_state_inout(state_in_out_t *item, uint32_t timeout_ticks);
    void send_state_mqtt_message( const char* key, const char* value);
    bool push_data_mqtt_set(const sMqttPkg_t *data, uint32_t timeout_ticks);
    bool pop_data_mqtt_set(sMqttPkg_t *data, uint32_t timeout_ticks);
    void handle_data_mqtt_set(sMqttPkg_t *mqttPkg);
    char* create_json_dynamic(const char *key, void *value, value_type_t type);
    void push_infor_version_ota(const char *version, char *status, char *fw_title);
    void publish_data_mqtt(const char *data);
#ifdef __cplusplus
}
#endif
// extern SemaphoreHandle_t xMutexMqttConfig;
// extern mqttConfig_t mqttConfig;
#endif