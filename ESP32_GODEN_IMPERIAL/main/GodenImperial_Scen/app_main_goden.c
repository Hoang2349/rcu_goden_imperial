#include "app_main_goden.h"

#include "Mqtt.h"
#include "app_control_output.h"
#include "app_goden_imperial_common.h"
#include "app_handle_input.h"
#include "app_state_persistence.h"
#include "app_advanced_state_management.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "app_nvs_config.h"

#define DEFAULT_TIME_CROSSING 30 * 60  // 30 minutes
#define DEFAULT_TIME_DOOR_AJAR 300      // 30 seconds

// define version

status_room_t status_room_cur;
status_room_t status_room_new = {false};
bool flag_syn_status_room = false;
pms_room_status_t pms_room_status;
status_sensor_t status_sensor;
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
    static uint32_t save_counter = 0;
    const uint32_t SAVE_INTERVAL = 600; // Save every 600 iterations (about 1 minute with 100ms delay)
    
    while (1)
    {
        if (flag_syn_status_room)
        {
            flag_syn_status_room = false;
            ESP_LOGI(__FUNCTION__, "Status room changed, updating...");
            
            // Save the current state to RAM when there's a change
            esp_err_t ret = save_current_state_to_ram();
            if (ret != ESP_OK) {
                ESP_LOGE(__FUNCTION__, "Failed to save state to RAM: %s", esp_err_to_name(ret));
            }
            
            // Also update the OCC state if we're currently in OCCUPIED state
            if (pms_room_status.status_human == OCCUPIED) {
                save_occ_initial_state();
            }
        }

        // Periodically save the state even if there are no changes
        save_counter++;
        if (save_counter >= SAVE_INTERVAL) {
            save_counter = 0;
            esp_err_t ret = save_current_state_to_ram();
            if (ret != ESP_OK) {
                ESP_LOGE(__FUNCTION__, "Periodic save failed: %s", esp_err_to_name(ret));
            }
            
            // Also periodically update the OCC state if we're currently in OCCUPIED state
            if (pms_room_status.status_human == OCCUPIED) {
                save_occ_initial_state();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // Delay 100ms
    }
}

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
                status_sensor.flag_motion_sensor = false;
                continue;
            }

            // staff mode - door opened
            if (flag_staff_mode == INACTIVE &&
                status_sensor.flag_door_sensor == DOOR_OPEN)
            {
                handle_scene_staff_mode();
                flag_staff_mode = ACTIVE;
                status_sensor.flag_motion_sensor = false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // --- RENTED ROOM ---
        if (pms_room_status.status_room == ROOM_RENTED)
        {
            ESP_LOGI(__FUNCTION__, "Waiting for door/motion sensor...: %ld",
                     time_crossing);
//////////////////////////////////////////////Door open///////////////////////////////////////
            // Door opened trig - welcome scene or return from standby, sự kiện của mở
            if (status_sensor.new_status_door_sensor &&
                status_sensor.flag_door_sensor == DOOR_OPEN)
            {
                
                time_crossing = time_crossing_set;
                status_sensor.new_status_door_sensor = false;
                status_sensor.flag_motion_sensor = false;
                 // initialize MQTT with default door ajar state
                char *json_string3 = create_json_dynamic("DOOR_AJAR", "false", TYPE_STRING);
                publish_data_mqtt(json_string3);
                free(json_string3);

                if (pms_room_status.flag_checkin_first)
                {
                    handle_scene_welcome();
                    pms_room_status.status_human = OCCUPIED;
                    
                    char *json_string2 = create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
                    publish_data_mqtt(json_string2);
                    free(json_string2);

                    pms_room_status.flag_checkin_first = false;
                    status_sensor.flag_motion_sensor = false;
                    char *json_string = create_json_dynamic(
                        "WELCOME_STATUS", "true", TYPE_STRING);
                    publish_data_mqtt(json_string);

                    free(json_string);
                    ESP_LOGW("Welcome check", "Guest checked in first time.");
                }
                else
                {
                    // Nếu không phải lần đầu tiên và đang ở trạng thái standby (UNOCCUPIED)
                    // Thì khôi phục trạng thái thiết bị khi mở cửa
                    if (pms_room_status.status_human == UNOCCUPIED)
                    {
                      
                        restore_occ_initial_state();
                        // Bật cả ITC và IDU khi khôi phục trạng thái
                        scene_itc_on();
                        scene_idu_on();

                        pms_room_status.status_human = OCCUPIED;

                        char *json_string4 = create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
                        publish_data_mqtt(json_string4);
                        free(json_string4);

                        status_sensor.flag_motion_sensor = false;
                        ESP_LOGW(__FUNCTION__, ">>> Restored device states from saved state (door opened)");
                    }
                    else
                    {
                        status_sensor.flag_motion_sensor = false;
                        ESP_LOGI(__FUNCTION__, "Door opened but room is not in standby, continuing normally");
                    }
                }
                continue;
            }

            // Door opened - just wait, cửa đang mở
            if (status_sensor.flag_door_sensor == DOOR_OPEN &&
                time_crossing > 0 && pms_room_status.flag_checkin_first == false &&  status_sensor.new_status_door_sensor == false)
            {
                status_sensor.flag_motion_sensor = false;
                vTaskDelay(pdMS_TO_TICKS(2000));
                status_sensor.flag_motion_sensor = false;
                vTaskDelay(pdMS_TO_TICKS(2000));
                ESP_LOGI(__FUNCTION__, "Door opened--------------");
                continue;
            }

//////////////////////////////////////////////Door close///////////////////////////////////////
            // Door closed - prepare to check motion, sự kiện cửa đóng
            if (status_sensor.new_status_door_sensor &&
                status_sensor.flag_door_sensor == DOOR_CLOSE)
            {
                check_mode = true;
                status_sensor.new_status_door_sensor = false;
                status_sensor.flag_motion_sensor = false;
                time_crossing = time_crossing_set;
                ESP_LOGI(__FUNCTION__, "Door closed, wait for motion");
                continue;
            }
           
            // Chương trinhg khi cửa đang đóng và kiểm tra chuyển động
            if (status_sensor.flag_door_sensor == DOOR_CLOSE &&
                time_crossing > 0 && pms_room_status.flag_checkin_first == false)
            {
                // neu có chuyển động trong thời gian chờ mà chưa UNOCCUPIED
                if (status_sensor.flag_motion_sensor &&
                    pms_room_status.status_human == UNOCCUPIED)
                {
                   
                    // Bật cả ITC và IDU khi khôi phục trạng thái
                    restore_occ_initial_state();
                    scene_itc_on();
                    scene_idu_on();
                    status_sensor.flag_motion_sensor = false;
                    ESP_LOGW(__FUNCTION__, ">>> Restored device states from saved state (door opened)");
                   

                    pms_room_status.status_human = OCCUPIED;
                    status_sensor.flag_motion_sensor = false;
                    check_mode = false;
                    time_crossing = time_crossing_set;

                    char *json_string = create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
                    publish_data_mqtt(json_string);
                    char *json_string1 =
                        create_json_dynamic("SET_BACK_ACTIVE", "false", TYPE_STRING);
                    publish_data_mqtt(json_string1);
                    free(json_string);
                    free(json_string1);
                    continue;
                }
                // neu có chuyển động trong thời gian chờ mà đang OCCUPIED
                if (status_sensor.flag_motion_sensor &&
                    pms_room_status.status_human == OCCUPIED)
                {
                    pms_room_status.status_human = OCCUPIED;
                    status_sensor.flag_motion_sensor = false;
                    check_mode = false;
                    time_crossing = time_crossing_set;

                    //char *json_string = create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
                    //publish_data_mqtt(json_string);
                    //char *json_string1 =
                    //    create_json_dynamic("SET_BACK_ACTIVE", "false", TYPE_STRING);
                    //publish_data_mqtt(json_string1);
                    //free(json_string);
                    //free(json_string1);
                    save_occ_initial_state();
                    // Cập nhật trạng thái đồng bộ
                    flag_syn_status_room = true;
                    continue;
                }
                // neu không có chuyển động trong thời gian chờ
                if (check_mode)
                {
                    ESP_LOGI(__FUNCTION__, "Handle timeout in rent");
                    time_crossing--;
                }
                
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
            
            // Handle unoccupied state
            if (!time_crossing)
            {

                if (pms_room_status.status_human == OCCUPIED)
                {
                    save_occ_initial_state();
                    flag_syn_status_room = true;
                }
                
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
    // Initialize the state persistence system first
    esp_err_t ret = init_state_persistence();
    if (ret != ESP_OK) {
        ESP_LOGW(__FUNCTION__, "State persistence initialization failed: %s", esp_err_to_name(ret));
        // Initialize with default values if persistence fails
        pms_room_status.status_room = ROOM_UNRENT;
        pms_room_status.flag_checkin_first = true;
        pms_room_status.flag_checkout_first = false;
        pms_room_status.flag_setback = false;
        pms_room_status.status_human = false;
        // init sensor state
        status_sensor.flag_door_sensor = DOOR_CLOSE;
        status_sensor.flag_motion_sensor = INACTIVE;
    }
    
    // Clear any saved OCC state at initialization
    clear_saved_occ_state();
    
    init_goden_imperial_input();
    app_nvs_config_t config_data = {0};
    if (goden_inperial_read_config_data(&config_data) == ESP_OK)
    {
        time_crossing_set = config_data.time_crossing;
        //time_door_ajar = config_data.time_door_ajar;
        time_door_ajar = DEFAULT_TIME_DOOR_AJAR;
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
