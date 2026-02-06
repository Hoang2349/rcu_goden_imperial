#include "Mqtt.h"

#include <stdlib.h>
#include <string.h>

#include "AppSpiffs.h"
#include "HttpsOtaRcu.h"
#include "MqttDeviceCommand.h"
#include "RuleEngine.h"
#include "app_control_output.h"
#include "app_goden_imperial_common.h"
#include "app_nvs_config.h"
#include "cJSON.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#define STATE_QUEUE_LENGTH 100

QueueHandle_t state_queue;
QueueHandle_t mqtt_set_queue;

static esp_mqtt_client_handle_t client;

sMqttPkg_t mqttPkg;

mqttConfig_t mqttConfig = {
    .mqttHost = MQTT_BROKER,
    .mqttPort = MQTT_PORT,
    .mqttUser = MQTT_USER,
    .mqttPass = MQTT_PASS,
};

void handleStateInOut(void *param)
{
    static state_in_out_t data_inout;
    while (1)
    {
        // handle date sub from MQTT
        if (pop_data_mqtt_set(&mqttPkg, 100 / portTICK_PERIOD_MS) == true)
        {
            ESP_LOGI(__FUNCTION__, "TOPIC: %s", mqttPkg.strTopic);
            ESP_LOGI(__FUNCTION__, "PAYLOAD: %s", mqttPkg.strPayload);
            // TODO: handle data sub from MQTT
            handle_data_mqtt_set(&mqttPkg);
            continue;
        }

        // handle data push to MQTT
        if (pop_state_inout(&data_inout, 100 / portTICK_PERIOD_MS) == true)
        {
            ESP_LOGI(__FUNCTION__, "Key: %s", data_inout.key);
            ESP_LOGI(__FUNCTION__, "Value: %s", data_inout.value);
            send_state_mqtt_message(data_inout.key, data_inout.value);
            goto wait_next;
        }
    wait_next:
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/// @brief Function log error when have event Error
/// @param message: Message log error
/// @param errorCode: code id return log terminal
static void logErrorIfNonZero(const char *message, int errorCode)
{
    if (errorCode != 0)
    {
        ESP_LOGE(__FUNCTION__, "Last error %s: 0x%x", message, errorCode);
    }
}

/// @brief Function handle any event mqtt on process
/// @param handlerArgs no use
/// @param base no use
/// @param eventId id event
/// @param eventData data event return
static void mqttEventHandler(void *handlerArgs, esp_event_base_t base,
                             int32_t eventId, void *eventData)
{
    esp_mqtt_event_handle_t event = eventData;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    sMqttPkg_t mqttPkg = {0};
    switch ((esp_mqtt_event_id_t)eventId)
    {
        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe_single(client, TOPIC_ATTRIBUTES_TOPIC,
                                             QOS_SET_LEVEL);
            esp_mqtt_client_subscribe_single(client, TOPIC_RPC_REQUEST_TOPIC,
                                             QOS_SET_LEVEL);
            esp_mqtt_client_subscribe_single(client, TOPIC_DEVICE_TELEMETRY,
                                             QOS_SET_LEVEL);

            push_infor_version_ota(versionEsp32, "UPDATED",
                                   DEFAULT_TITLE_GODEN_IMPERIAL);
            publish_data_mqtt("{\"fw_state\": \"UPDATED\"}");
            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY,
                                    "{\"OFS\":\"ONLINE\"}", 0, QOS_SET_LEVEL,
                                    0);
        }
        break;
        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGW(__FUNCTION__, "MQTT_EVENT_DISCONNECTED");
        }
        break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_SUBSCRIBED, msg_id=%d",
                     event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d",
                     event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:

            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_PUBLISHED, msg_id=%d",
                     event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_DATA");
            ESP_LOGI(__FUNCTION__, "Free memory: %ld bytes",
                     esp_get_free_heap_size());
            ESP_LOGW(__FUNCTION__,
                     "==================EVENT DATA===================");

            mqttPkg.lenTopic = event->topic_len;
            mqttPkg.lenPayload = event->data_len;
            strncpy(mqttPkg.strTopic, event->topic, mqttPkg.lenTopic);
            strncpy(mqttPkg.strPayload, event->data, mqttPkg.lenPayload);
            push_data_mqtt_set(&mqttPkg, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "\033[0;37m TOPIC:------> %.*s \033[0m",
                     event->topic_len, mqttPkg.strTopic);
            ESP_LOGI(__FUNCTION__, "\033[0;37m PAYLOAD:-----> %.*s \033[0m",
                     event->data_len, mqttPkg.strPayload);
            ESP_LOGW(__FUNCTION__,
                     "=============================================");
            // free(event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(__FUNCTION__, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type ==
                MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                logErrorIfNonZero("reported from esp-tls",
                                  event->error_handle->esp_tls_last_esp_err);
                logErrorIfNonZero("reported from tls stack",
                                  event->error_handle->esp_tls_stack_err);
                logErrorIfNonZero(
                    "captured as transport's socket errno",
                    event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(
                    __FUNCTION__, "Last errno string (%s)",
                    strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(__FUNCTION__, "Other event id:%d", event->event_id);
            break;
    }
}

/// @brief setup config MQTT and start mqtt client
void appMqttInit()
{
    // INIT QUEUE FOR STATE IN OUT
    state_queue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(state_in_out_t));
    if (state_queue == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Create queue state in out error!!!");
        return;
    }

    // INIT QUEUE FOR MQTT SET
    mqtt_set_queue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(sMqttPkg_t));
    if (mqtt_set_queue == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Create queue mqtt set error!!!");
        return;
    }
    // Creat task for handle state in out
    xTaskCreate(handleStateInOut, "handleStateInOut", 4096, NULL, 6, NULL);

    // Get config mqtt from spiffs
    if (getMqttConfig(mqttConfig.mqttHost, &mqttConfig.mqttPort,
                      mqttConfig.mqttUser) == false)
    {
        ESP_LOGE(__FUNCTION__, "Get config mqtt error!!!");
        return;
    }
    ESP_LOGI(__FUNCTION__, "MQTT CONFIG:");
    ESP_LOGI(__FUNCTION__, "broker: %s", mqttConfig.mqttHost);
    ESP_LOGI(__FUNCTION__, "port: %d", mqttConfig.mqttPort);
    ESP_LOGI(__FUNCTION__, "username: %s", mqttConfig.mqttUser);
    strcpy(accessTokenEsp32, mqttConfig.mqttUser);

    esp_mqtt_client_config_t mqttClientConfig = {
        .broker.address.uri = mqttConfig.mqttHost,
        .broker.address.port = mqttConfig.mqttPort,
        .credentials.username = mqttConfig.mqttUser,
        .session.keepalive = MQTT_KEEPALIVE,
    };
    client = esp_mqtt_client_init(&mqttClientConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqttEventHandler,
                                   client);
    esp_mqtt_client_start(client);
}

/// @brief Publish data to mqtt server
/// @param topic topic need send
/// @param payload data send format sparkplug
/// @param PayloadLen length payload
void appMqttPublish(char *topic, char *payload, int payloadLen)
{
    // ESP_LOGI(__FUNCTION__, "Topic Publish: %s", topic);
    esp_mqtt_client_publish(client, (const char *)topic, (const char *)payload,
                            payloadLen, QOS_SET_LEVEL, RETAIN_SET_DISABLE);
}

/// @brief coppy array to new address
/// @param des dstinations address
/// @param src source address need coppy
/// @param pStartReadSrc locations start coppy
/// @param len length need coppy
void cpyArray(char *des, char *src, uint16_t pStartReadSrc, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        *des = *(src + pStartReadSrc);
        des++;
        src++;
    }
}

bool push_state_inout(const state_in_out_t *item, uint32_t timeout_ticks)
{
    return xQueueSend(state_queue, item, timeout_ticks) == pdPASS;
}
bool pop_state_inout(state_in_out_t *item, uint32_t timeout_ticks)
{
    return xQueueReceive(state_queue, item, timeout_ticks) == pdPASS;
}

bool push_data_mqtt_set(const sMqttPkg_t *data, uint32_t timeout_ticks)
{
    return xQueueSend(mqtt_set_queue, data, timeout_ticks) == pdPASS;
}
bool pop_data_mqtt_set(sMqttPkg_t *data, uint32_t timeout_ticks)
{
    return xQueueReceive(mqtt_set_queue, data, timeout_ticks) == pdPASS;
}

void send_state_mqtt_message(const char *key, const char *value)
{
    // Tạo đối tượng JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, key, value);

    // Chuyển đổi JSON sang chuỗi
    char *json_string = cJSON_Print(root);
    // ESP_LOGI("MQTT", "Sending message: %s", json_string);

    // Gửi thông điệp MQTT
    esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string, 0,
                            QOS_SET_LEVEL, 0);

    // Giải phóng bộ nhớ
    cJSON_Delete(root);
    free(json_string);
}

void handle_data_mqtt_set(sMqttPkg_t *mqttPkg)
{
    if (strncmp(mqttPkg->strTopic, TOPIC_ATTRIBUTES_TOPIC,
                strlen(TOPIC_ATTRIBUTES_TOPIC)) == 0)
    {
        ESP_LOGI(__FUNCTION__, "TOPIC: %s", mqttPkg->strTopic);
        ESP_LOGI(__FUNCTION__, "PAYLOAD: %s", mqttPkg->strPayload);
        // Parse data from mqtt to json
        cJSON *root = cJSON_Parse(mqttPkg->strPayload);
        if (root == NULL)
        {
            ESP_LOGE(__FUNCTION__, "Error parse json data!!!");
            cJSON_Delete(root);
            return;
        }

        cJSON *idu = cJSON_GetObjectItem(root, "IDU_ATT");
        cJSON *pms = cJSON_GetObjectItem(root, "PMS_STATUS_ATT");
        cJSON *roomStatus = cJSON_GetObjectItem(root, "ROOM_STATUS_ATT");
        cJSON *timeoutUnoccupied = cJSON_GetObjectItem(root, "UNOCC_DELAY_ATT");
        cJSON *timeout_door_ajar = cJSON_GetObjectItem(root, "DOOR_AJAR_ATT");

        // handle data idu
        if (idu != NULL)
        {
            ESP_LOGI(__FUNCTION__, "IDU_ATT: %d", idu->valueint);
            char *json_string =
                create_json_dynamic("IDU", &idu->valueint, TYPE_BOOL);
            if (json_string == NULL)
            {
                ESP_LOGE(__FUNCTION__, "Error create json string!!!");
                cJSON_Delete(root);
                return;
            }
            // set status IDU
            idu->valueint == 1 ? scene_idu_on() : scene_idu_off();

            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string,
                                    0, QOS_SET_LEVEL, 0);
            cJSON_Delete(root);
            return;
        }

        // handle data PMS
        if (pms != NULL)
        {
            ESP_LOGI(__FUNCTION__, "PMS_STATUS_ATT: %d", pms->valueint);
            char *json_string =
                create_json_dynamic("PMS", &pms->valueint, TYPE_BOOL);
            if (json_string == NULL)
            {
                ESP_LOGE(__FUNCTION__, "Error create json string!!!");
                cJSON_Delete(root);
                return;
            }
            // set status PMS
            if (pms->valueint == ROOM_RENTED)
            {
                pms_room_status.status_room = ROOM_RENTED;
                pms_room_status.flag_checkin_first = true;
                char *json_string =
                    create_json_dynamic("ROOM_STATUS", "RENTED", TYPE_STRING);
                // publish_data_mqtt(json_string);
                esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY,
                                        json_string, 0, QOS_SET_LEVEL, 1);
                char *json_string_1 =
                    create_json_dynamic("WELCOME_STATUS", "false", TYPE_STRING);
                esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY,
                                        json_string_1, 0, QOS_SET_LEVEL, 0);
                free(json_string_1);
                free(json_string);
            }
            else
            {
                // set status room unrented
                pms_room_status.status_room = ROOM_UNRENT;
                handle_scene_unrented();
            }
            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string,
                                    0, QOS_SET_LEVEL, 1);
            cJSON_Delete(root);
            return;
        }

        // handle data ROOM_STATUS_ATT
        if (roomStatus != NULL)
        {
            ESP_LOGI(__FUNCTION__, "ROOM_STATUS_ATT: %s",
                     roomStatus->valuestring);
            char *value = roomStatus->valuestring;
            ESP_LOGI(__FUNCTION__, "ROOM_STATUS_ATT: %s", value);
            char *json_string =
                create_json_dynamic("ROOM_STATUS", value, TYPE_STRING);
            if (json_string == NULL)
            {
                ESP_LOGE(__FUNCTION__, "Error create json string!!!");
                cJSON_Delete(root);
                return;
            }
            // handle status room
            if (pms_room_status.status_room == ROOM_UNRENT)
            {
                if (strcmp(value, "STANDBY") == 0)
                {
                    handle_scene_standby();
                }
                else if (strcmp(value, "UNRENTED") == 0)
                {
                    handle_scene_unrented();
                }
                if (strcmp(value, "OCC") == 0)
                {
                    handle_scene_occupied();
                }
                else if (strcmp(value, "UNOCC") == 0)
                {
                    handle_scene_standby();
                }
                else if (strcmp(value, "STAFF") == 0)
                {
                    handle_scene_staff_mode();
                }
            }
            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string,
                                    0, QOS_SET_LEVEL, 0);
            cJSON_Delete(root);
            return;
        }

        // SET TIMEOUT UNOCCUPIED
        if (timeoutUnoccupied != NULL)
        {
            ESP_LOGI(__FUNCTION__, "UNOCC_DELAY_ATT: %f",
                     timeoutUnoccupied->valuedouble);
            char *json_string = create_json_dynamic(
                "UNOCC_DELAY", &timeoutUnoccupied->valuedouble, TYPE_DOUBLE);
            uint32_t new_timeout = (uint32_t)timeoutUnoccupied->valuedouble;
            if (new_timeout != time_crossing_set)
            {
                time_crossing_set = new_timeout;
                printf("TIMEOUT UNOCCUPIED: %ld\n", time_crossing_set);
                app_nvs_config_t config_data = {time_crossing_set,
                                                time_door_ajar};
                goden_inperial_write_config_data(&config_data);
            }
            else
            {
                printf("Giá trị timeout không thay đổi, không cần lưu vào NVS\n");
            }

            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string,
                                    0, QOS_SET_LEVEL, 0);
            cJSON_Delete(root);
            return;
        }

        if (timeout_door_ajar != NULL)
        {
            ESP_LOGI(__FUNCTION__, "DOOR_AJAR: %f",
                     timeout_door_ajar->valuedouble);
            char *json_string = create_json_dynamic(
                "DOOR_AJAR", &timeout_door_ajar->valuedouble, TYPE_INT);
            if (pms_room_status.status_room == ROOM_UNRENT)
            {
                time_door_ajar = timeout_door_ajar->valuedouble;
                app_nvs_config_t config_data = {time_crossing_set,
                                                time_door_ajar};
                goden_inperial_write_config_data(&config_data);
                printf("TIMEOUT DOOR AJAR: %f\n", time_door_ajar);
            }
            esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, json_string,
                                    0, QOS_SET_LEVEL, 0);
            cJSON_Delete(root);
            return;
        }
    }
}

char *create_json_dynamic(const char *key, void *value, value_type_t type)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return NULL;
    }

    switch (type)
    {
        case TYPE_STRING:
            cJSON_AddStringToObject(root, key, (const char *)value);
            break;
        case TYPE_INT:
            cJSON_AddNumberToObject(root, key, *((int *)value));
            break;
        case TYPE_BOOL:
            cJSON_AddBoolToObject(root, key, *((bool *)value));
            break;
        case TYPE_DOUBLE:
            cJSON_AddNumberToObject(root, key, *((double *)value));
            break;
        default:
            cJSON_Delete(root);
            return NULL;
    }

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

void push_infor_version_ota(const char *version, char *status, char *fw_title)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }
    cJSON_AddStringToObject(root, "fw_version", version);
    cJSON_AddStringToObject(root, "fw_state", status);
    cJSON_AddStringToObject(root, "fw_title", fw_title);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    esp_mqtt_client_publish(client, TOPIC_ATTRIBUTES_TOPIC, json_str, 0,
                            QOS_SET_LEVEL, 0);
    free(json_str);
}

void publish_data_mqtt(const char *data)
{
    esp_mqtt_client_publish(client, TOPIC_DEVICE_TELEMETRY, data, 0,
                            QOS_SET_LEVEL, 0);
    ESP_LOGI(__FUNCTION__, "Publish data: %s", data);
}