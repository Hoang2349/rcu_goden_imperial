#include "app_main_goden.h"

#include "Mqtt.h"
#include "app_control_output.h"
#include "app_goden_imperial_common.h"
#include "app_handle_input.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "app_nvs_config.h"

#define DEFAULT_TIME_CROSSING 30 * 60  // 30 minutes
#define DEFAULT_TIME_DOOR_AJAR 6      // 6 seconds

// define version

status_room_t status_room_cur;
status_room_t status_room_new = {false};
bool flag_syn_status_room = false;
pms_room_status_t pms_room_status;
status_sensor_t status_sensor;
state_setback_t state_setback;
status_outdoor_t status_outdoor = {false};

uint32_t time_crossing = DEFAULT_TIME_CROSSING;
uint32_t time_crossing_set = DEFAULT_TIME_CROSSING;
double time_door_ajar = DEFAULT_TIME_DOOR_AJAR;
uint8_t flag_staff_mode = INACTIVE;

void init_status_room()
{
    status_room_cur.master_m1_status = false;
    status_room_cur.toilet_status = false;
    status_room_cur.wc_light_status = false;
    status_room_cur.minibar_status = false;
    status_room_cur.master_m2_status = false;
    status_room_cur.reading_s2_status = false;
    status_room_cur.ceiling_light_s2_status = false;
    status_room_cur.night_light_s2_status = false;
    status_room_cur.night_light_s3_status = false;
    status_room_cur.master_m3_status = false;
    status_room_cur.ceiling_light_s3_status = false;
    status_room_cur.reading_s3_status = false;
}

void syn_status_room(void *param)
{
    while (1)
    {
        if (flag_syn_status_room)
        {
            flag_syn_status_room = false;
            ESP_LOGI(__FUNCTION__, "Status room changed, updating...");
            handle_led_status();
            update_setback_status_room();
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // Delay 1 giây
    }
}

bool temp = false;
bool check_mode = false;
void rule_room_hotel()
{
    while (1)
    {
        // --- UNRENTED ROOM ---
        if (pms_room_status.status_room == ROOM_UNRENT)
        {
            if (status_sensor.flag_door_sensor == DOOR_CLOSE &&
                flag_staff_mode == ACTIVE)
            {
                vTaskDelay(pdMS_TO_TICKS(30000));
                handle_scene_unrented();
                flag_staff_mode = INACTIVE;
                continue;
            }

            // staff mode - door opened
            if (flag_staff_mode == INACTIVE &&
                status_sensor.flag_door_sensor == DOOR_OPEN)
            {
                handle_scene_staff_mode();
                flag_staff_mode = ACTIVE;
            }

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // --- RENTED ROOM ---
        if (pms_room_status.status_room == ROOM_RENTED)
        {
            ESP_LOGI(__FUNCTION__, "Waiting for door/motion sensor...: %ld",
                     time_crossing);

            // Door opened - welcome scene
            if (status_sensor.new_status_door_sensor &&
                status_sensor.flag_door_sensor == DOOR_OPEN)
            {
                time_crossing = time_crossing_set;
                status_sensor.new_status_door_sensor = false;

                if (pms_room_status.flag_checkin_first)
                {
                    handle_scene_welcome();
                    pms_room_status.status_human = OCCUPIED;
                    pms_room_status.flag_checkin_first = false;
                    temp = true;
                    char *json_string = create_json_dynamic(
                        "WELCOME_STATUS", "true", TYPE_STRING);
                    publish_data_mqtt(json_string);

                    free(json_string);
                    ESP_LOGW("Welcome check", "Guest checked in first time.");
                }
                else
                {
                    // Nếu không phải lần đầu tiên và đang ở trạng thái standby (UNOCCUPIED)
                    // Không áp dụng lại trạng thái setback khi cửa mở, chỉ áp dụng khi có chuyển động
                    // Trạng thái setback sẽ được áp dụng khi phát hiện chuyển động và chuyển sang trạng thái OCCUPIED
                    ESP_LOGI(__FUNCTION__, "Door opened but room is in standby (UNOCCUPIED), waiting for motion to apply setback");
                }
                continue;
            }

            // Door closed - prepare to check motion
            if (status_sensor.new_status_door_sensor &&
                status_sensor.flag_door_sensor == DOOR_CLOSE)
            {
                check_mode = true;
                status_sensor.new_status_door_sensor = false;
                temp = false;
                time_crossing = time_crossing_set;
                ESP_LOGI(__FUNCTION__, "Door closed, wait for motion");
                continue;
            }

            // Handle timeout if no motion detected after welcome
            if (temp)
            {
                if (time_crossing > 0)
                {
                    time_crossing--;
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                }
                temp = false;
                time_crossing = time_crossing_set;
                handle_scene_standby();
            }

            // Check for occupied status
            if (status_sensor.flag_door_sensor == DOOR_CLOSE &&
                time_crossing > 0)
            {
                if (status_sensor.flag_motion_sensor &&
                    pms_room_status.status_human == UNOCCUPIED)
                {
                    pms_room_status.status_human = OCCUPIED;
                    status_sensor.flag_motion_sensor = false;
                    check_mode = false;
                    time_crossing = time_crossing_set;
                    handle_scene_occupied();
                    ESP_LOGW(__FUNCTION__, ">>> OCCUPIED");
                    continue;
                }

                if (status_sensor.flag_motion_sensor &&
                    pms_room_status.status_human == OCCUPIED)
                {
                    status_sensor.flag_motion_sensor = false;
                    check_mode = false;
                    time_crossing = time_crossing_set;
                }

                if (pms_room_status.status_human == OCCUPIED && check_mode)
                {
                    time_crossing--;
                }

                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // Handle unoccupied state
            if (!time_crossing)
            {
                pms_room_status.status_human = UNOCCUPIED;
                status_sensor.flag_motion_sensor = false;
                check_mode = false;
                time_crossing = time_crossing_set;
                handle_scene_standby();
                char *json_string =
                    create_json_dynamic("SET_BACK_ACTIVE", "true", TYPE_STRING);
                publish_data_mqtt(json_string);
                free(json_string);
                ESP_LOGW(__FUNCTION__, ">>> UNOCCUPIED");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void init_goden_imperial()
{
    // Initialize the status room
    pms_room_status.status_room = ROOM_UNRENT;
    pms_room_status.flag_checkin_first = true;
    pms_room_status.flag_checkout_first = false;
    pms_room_status.flag_setback = false;
    pms_room_status.status_human = false;

    // init sensor state
    status_sensor.flag_door_sensor = DOOR_CLOSE;
    status_sensor.flag_motion_sensor = INACTIVE;

    // INIT STATE SETBACK
    for (int i = 0; i < MAX_RELAY_CONTROL; i++)
    {
        state_setback.relay[i].index = RELAY_C1 + i;
        state_setback.relay[i].value = 0X00;
    }

    state_setback.led[0].index = LED_MASTER_M1;
    state_setback.led[0].value = 0X00;
    state_setback.led[1].index = LED_MASTER_M2;
    state_setback.led[1].value = 0X00;
    state_setback.led[2].index = LED_MASTER_M3;
    state_setback.led[2].value = 0X00;
    state_setback.led[3].index = LED_TOILET;
    state_setback.led[3].value = 0X00;
    state_setback.led[4].index = LED_BATHROOM;
    state_setback.led[4].value = 0X00;
    state_setback.led[5].index = LED_MINIBAR;
    state_setback.led[5].value = 0X00;
    state_setback.led[6].index = LED_SPORT_LIGHT;
    state_setback.led[6].value = 0X00;
    state_setback.led[7].index = LED_DECORATION_S2;
    state_setback.led[7].value = 0X00;
    state_setback.led[8].index = LED_DECORATION_S3;
    state_setback.led[8].value = 0X00;
    state_setback.led[9].index = LED_COVER_LIGHT;
    state_setback.led[9].value = 0X00;
    state_setback.led[10].index = LED_READING_S2;
    state_setback.led[10].value = 0X00;
    state_setback.led[11].index = LED_READING_S3;
    state_setback.led[11].value = 0X00;
    init_goden_imperial_input();
    app_nvs_config_t config_data = {0};
    if (goden_inperial_read_config_data(&config_data) == ESP_OK)
    {
        time_crossing_set = config_data.time_crossing;
        time_door_ajar = config_data.time_door_ajar;
        ESP_LOGI(__FUNCTION__, "Loaded config data from NVS: time_crossing=%ld, time_door_ajar=%f",
                 time_crossing_set, time_door_ajar);
    }
    else
    {
        ESP_LOGW(__FUNCTION__, "No config data found in NVS, using defaults.");
        time_crossing_set = DEFAULT_TIME_CROSSING;
        time_door_ajar = DEFAULT_TIME_DOOR_AJAR;
    }
    xTaskCreate(syn_status_room, "Task syn status switch", 4086, NULL, 5, NULL);
    xTaskCreate(rule_room_hotel, "Rule hotel management", 4086, NULL, 6, NULL);
}
