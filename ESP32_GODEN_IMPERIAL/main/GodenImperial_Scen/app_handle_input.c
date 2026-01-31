#include "app_handle_input.h"
// #include "queueInputStm32.h"
#include "Modbus.h"
#include "Mqtt.h"
#include "app_control_output.h"
#include "app_goden_imperial_common.h"

#define TIMEOUT_AFTER_BELL_ACTIVE (3000)  // 3 SECONDS
QueueHandle_t queueInputStm32;

/**
 * @brief Ham khoi tao queue input tu STM32
 * @param None
 * @return None
 */
void queueInputStm32Init()
{
    queueInputStm32 =
        xQueueCreate(MAX_QUEUE_INPUT_STM32, MAX_LENGTH_INPUT_STM32);
    if (queueInputStm32 == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Error create queue");
        esp_restart();
    }
}

/**
 * @brief Ham truyen ouput vao queue
 * @param variableOutput_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueInputStm32Push(const uint8_t *input)
{
    if (xQueueSendToBack(queueInputStm32, (void *)input,
                         (TickType_t)TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        // ESP_LOGE(__FUNCTION__, "Error xQueueSendToBack = errQUEUE_FULL");
        return QUEUE_FAIL;
    }
    else
    {
        ESP_LOGI(__FUNCTION__, "Push data Success");
        return QUEUE_OK;
    }
}
/**
 * @brief Ham truyen lay output tu queue
 * @param variableOutput_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueInputStm32Pop(uint8_t *output)
{
    if (xQueueReceive(queueInputStm32, output,
                      (TickType_t)TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        // ESP_LOGE(__FUNCTION__, "Error xQueueRecive Queue Empty!");
        return QUEUE_FAIL;
    }
    else
    {
        ESP_LOGI(__FUNCTION__, "Pop data Success");
        return QUEUE_OK;
    }
}

void process_handle_input()
{
    while (1)
    {
        uint8_t data[MAX_LENGTH_INPUT_STM32] = {0};
        if (queueInputStm32Pop(data) == QUEUE_OK)
        {
            ESP_LOGI(__FUNCTION__, "Data from queue:");
            ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, data, MAX_LENGTH_INPUT_STM32,
                                     ESP_LOG_INFO);
            // HANDLE button mapping scenario
            check_active_scen(data[5], data[4]);
        }
        else
        {
            // ESP_LOGE(__FUNCTION__, "Failed to pop data from queue");
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Delay 1 giây
    }
}

bool temp1 = false;
void hanlde_mode_outdoor(void *param)
{
    while (1)
    {
        static uint16_t count = 0;
        // if DND enable disable MUR and Bell
        vTaskDelay(pdMS_TO_TICKS(100));  // Delay 1 giây
        if (status_outdoor.status_bell)
        {
            scene_bell();
            status_outdoor.status_bell = false;
            vTaskDelay(pdMS_TO_TICKS(TIMEOUT_AFTER_BELL_ACTIVE));
        }
        // dooor ajar
        if (status_sensor.flag_door_sensor == DOOR_OPEN)
        {
            if (count > (time_door_ajar * 10) && temp1 == false)
            {
                char *json_string = "{\"DOOR_AJAR\": \"true\"}";
                publish_data_mqtt(json_string);
                temp1 = true;
            }
            count++;
            continue;
        }
        if (status_sensor.flag_door_sensor == DOOR_CLOSE && temp1 == true)
        {
            count = 0;
            temp1 = false;
            char *json_string = "{\"DOOR_AJAR\": \"false\"}";
            publish_data_mqtt(json_string);
        }
    }
}

void init_goden_imperial_input()
{
    queueInputStm32Init();
    xTaskCreate(process_handle_input,       // Task function
                "Task handle input STm32",  // Tên task
                4048,   // Stack size (bytes / 4 = 512 words)
                NULL,   // Tham số truyền vào
                5,      // Priority
                NULL);  // Không cần handle
    xTaskCreate(hanlde_mode_outdoor, "handle bell", 4096, NULL, 7, NULL);
    xTaskCreate(handleRs485, "Rs485 Handle TX-Rx", 4096, NULL, 6, NULL);
}

void check_active_scen(uint8_t pin_active, uint8_t status)
{
    ESP_LOGW(__FUNCTION__, "Pin active: %d", pin_active);
    bool new_status = false;
    state_in_out_t state_inout = {0};
    switch (pin_active)
    {
        case EVENT_BELL:
            ESP_LOGI(__FUNCTION__, "Bell event triggered");
            if (!status_outdoor.status_dnd && !status_outdoor.status_bell)
            {
                status_outdoor.status_bell = true;
            }
            break;

        case EVENT_DND:
            ESP_LOGI(__FUNCTION__, "DND event triggered");
            status_outdoor.status_dnd = !status_outdoor.status_dnd;
            scene_dnd(status_outdoor.status_dnd);
            break;

        case EVENT_MUR:
            ESP_LOGI(__FUNCTION__, "MUR event triggered");
            status_outdoor.status_mur = !status_outdoor.status_mur;
            scene_mur(status_outdoor.status_mur);
            break;

        case EVENT_DOOR:
            ESP_LOGI(__FUNCTION__, "Door event triggered");
            status_sensor.flag_door_sensor = status;
            strcpy(state_inout.key, "DOOR");
            strcpy(state_inout.value, status ? "false" : "true");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            status_sensor.new_status_door_sensor = true;
            break;

        case EVENT_MOTION:
            ESP_LOGI(__FUNCTION__, "Motion event triggered");
            status_sensor.flag_motion_sensor = status;
            strcpy(state_inout.key, "OCCSEN");
            strcpy(state_inout.value, status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            // TODO: add handle reset motion sensor AFETR TIMEOUT
            break;

        case EVENT_MASTER_M1:
            new_status = !status_room_cur.master_m1_status;
            handle_event_master_m1(new_status);
            strcpy(state_inout.key, "S1_M1");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Master M1 event triggered");
            break;

        case EVENT_TOILET:
            new_status = !status_room_cur.toilet_status;
            strcpy(state_inout.key, "S1A_TOILET");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Toilet event triggered");
            handle_event_toilet(new_status);
            break;

        case EVENT_WC_LIGHTING:
            new_status = !status_room_cur.wc_light_status;
            strcpy(state_inout.key, "S1A_WC");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            handle_event_wc_light(new_status);
            ESP_LOGI(__FUNCTION__, "WC Lighting event triggered");
            break;

        case EVENT_MINIBAR:
            new_status = !status_room_cur.minibar_status;
            handle_event_minibar(new_status);
            strcpy(state_inout.key, "S4_MINIBAR");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Minibar event triggered");
            break;

        case EVENT_MASTER_M2:
            new_status = !status_room_cur.master_m2_status;
            handle_event_master_m2(new_status);
            strcpy(state_inout.key, "S2_M2");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Master M2 event triggered");
            break;

        case EVENT_READING_S2:
            new_status = !status_room_cur.reading_s2_status;
            handle_event_reading_s2(new_status);
            strcpy(state_inout.key, "S2_READ");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Reading S2 event triggered");
            break;

        case EVENT_CEILING_S2:
            new_status = !status_room_cur.ceiling_light_s2_status;
            handle_event_ceiling_s2(new_status);
            strcpy(state_inout.key, "S2_CEILING");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Ceiling S2 event triggered");
            break;
            
        case EVENT_NIGHT_LIGHT_S2:
            new_status = !status_room_cur.night_light_s2_status;
            handle_event_night_s2(new_status);
            strcpy(state_inout.key, "S2_NIGHT");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Night Light S2 event triggered");
            break;

        case EVENT_NIGHT_LIGHT_S3:
            new_status = !status_room_cur.night_light_s3_status;
            handle_event_night_s3(new_status);
            strcpy(state_inout.key, "S3_NIGHT");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Night Light S3 event triggered");
            break;
            
        case EVENT_CEILING_S3:
            ESP_LOGI(__FUNCTION__, "Ceiling S3 event triggered");
            new_status = !status_room_cur.ceiling_light_s3_status;
            handle_event_ceiling_s3(new_status);
            strcpy(state_inout.key, "S3_CEILING");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            break;
            
        case EVENT_READING_S3:
            ESP_LOGI(__FUNCTION__, "Reading S3 event triggered");
            new_status = !status_room_cur.reading_s3_status;
            handle_event_reading_s3(new_status);
            strcpy(state_inout.key, "S3_READ");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            break;

        case EVENT_MASTER_M3:
            new_status = !status_room_cur.master_m3_status;
            handle_event_master_m3(new_status);
            strcpy(state_inout.key, "S3_M3");
            strcpy(state_inout.value, new_status ? "true" : "false");
            push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
            ESP_LOGI(__FUNCTION__, "Master M3 event triggered");
            break;

        default:
            ESP_LOGI(__FUNCTION__, "Unknown event triggered: %d", pin_active);
            break;
    }
}