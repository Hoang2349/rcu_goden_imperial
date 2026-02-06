#include "app_control_output.h"

#include <string.h>

#include "Modbus.h"
#include "Mqtt.h"
#include "app_goden_imperial_common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define TIME_ACTIVE_BELL (500)  // 500ms

// Global variable to track if we're currently restoring from setback
static bool is_restoring_from_setback = false;

#define HANDLE_LED_STATUS(field, led_func, led_index)                \
    do                                                               \
    {                                                                \
        if (status_room_cur.field != status_room_new.field)          \
        {                                                            \
            status_room_cur.field = status_room_new.field;           \
            led_func(led_index,                                      \
                     status_room_cur.field); /* Update LED status */ \
        }                                                            \
    } while (0)

void generate_led_command(uint8_t led_index, bool turn_on, char *output)
{
    const char template_data[MAX_DATA_SIZE] = {
        0x09, 0x10, 0x00, 0x01, 0x04, 0x08, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(output, template_data, MAX_DATA_SIZE);

    output[9] = led_index;               // LED number
    output[10] = turn_on ? 0xFF : 0x00;  // On/off value
}

void generate_relay_command(uint8_t relay_index, bool turn_on, char *output)
{
    const char template_data[MAX_DATA_SIZE] = {
        0x09, 0x10, 0x00, 0x01, 0x04, 0x08, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(output, template_data, MAX_DATA_SIZE);

    output[7] = relay_index;             // Relay number
    output[10] = turn_on ? 0xFF : 0x00;  // On/off value
}

typedef void (*scene_func_t)(void);

static void set_scene_status(bool status_flag, scene_func_t on_func,
                             scene_func_t off_func)
{
    if (status_flag)
        on_func();
    else
        off_func();
}

void scene_goden_init(void)
{
    // Initialize the scene for the Goden Imperial
    // This function should set up any necessary configurations or states
    // for the Goden Imperial scene.
}
void scene_goden_deinit(void)
{
    // Deinitialize the scene for the Goden Imperial
    // This function should clean up any resources or states used by the scene.
}

// Scene control functions

void scene_master_m1_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, true);
}
void scene_master_m1_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, false);
}

void scene_master_m2_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, true);
}

void scene_master_m2_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, false);
}

void scene_master_m3_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, true);
}
void scene_master_m3_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, false);
}

void scene_toilet_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C2, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C2, true);
}

void scene_toilet_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C2, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C2, false);
}

void scene_wc_light_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C3, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C3, true);
}

void scene_wc_light_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C3, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C3, false);
}
void scene_reading_s2_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C6, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C6, true);
    generate_relay_command(RELAY_C11, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C11, true);
}
void scene_reading_s2_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C6, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C6, false);
    generate_relay_command(RELAY_C11, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C11, false);
}
void scene_ceiling_s2_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C7, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C7, true);
}

void scene_ceiling_s2_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C7, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C7, false);
}

void scene_night_light_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C5, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C5, true);
}

void scene_night_light_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C5, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C5, false);
}

void scene_ceiling_s3_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C4, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C4, true);
}

void scene_ceiling_s3_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C4, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C4, false);
}

void scene_reading_s3_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C8, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C8, true);
    generate_relay_command(RELAY_C12, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C12, true);
}
void scene_reading_s3_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C8, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C8, false);
    generate_relay_command(RELAY_C12, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C12, false);
}

void scene_minibar_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C9, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C9, true);
}

void scene_minibar_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C9, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C9, false);
}

void scene_bell_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, true);  // Added push state for bell on
    generate_relay_command(RELAY_C10, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C10, true);
}

void scene_bell_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C1, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C1, false);  // Added push state for bell off
    generate_relay_command(RELAY_C10, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C10, false);
}

void scene_itc_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_ICT, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);

    push_state_inout_mqtt(RELAY_ICT, true);  // Added push state for ITC on
}
void scene_itc_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_ICT, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);

    push_state_inout_mqtt(RELAY_ICT, false);  // Added push state for ITC off
}
void scene_idu_on()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_IDU, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_IDU, true);  // Added push state for IDU on
}
void scene_idu_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_IDU, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_IDU, false);  // Added push state for IDU off
}

// Event Handlers
void handle_event_master_m1(bool new_status)
{
    // bool new_status = !status_room_cur.master_m1_status;
    set_scene_status(new_status, scene_master_m1_on, scene_master_m1_off);
    status_room_new.master_m1_status = new_status;
    status_room_new.master_m2_status = new_status;
    status_room_new.master_m3_status = new_status;
    // Update the MQTT state for master M1
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S1_M1");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S2_M2
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S2_M2");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S3_M3
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S3_M3");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_master_m2(bool new_status)
{
    set_scene_status(new_status, scene_master_m2_on, scene_master_m2_off);
    status_room_new.master_m1_status = new_status;
    status_room_new.master_m2_status = new_status;
    status_room_new.master_m3_status = new_status;


    // Update the MQTT state for master M2
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S1_M1");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S2_M2
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S2_M2");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S3_M3
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S3_M3");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_toilet(bool new_status)
{
    set_scene_status(new_status, scene_toilet_on, scene_toilet_off);
    status_room_new.toilet_status = new_status;
    
    // Update the MQTT state for toilet
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S1A_TOILET");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_wc_light(bool new_status)
{
    set_scene_status(new_status, scene_wc_light_on, scene_wc_light_off);
    status_room_new.wc_light_status = new_status;
    
    // Update the MQTT state for WC light
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S1A_WC");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_minibar(bool new_status)
{
    set_scene_status(new_status, scene_minibar_on, scene_minibar_off);
    status_room_new.minibar_status = new_status;
    
    // Update the MQTT state for minibar
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S4_MINIBAR");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_reading_s2(bool new_status)
{
    set_scene_status(new_status, scene_reading_s2_on, scene_reading_s2_off);
    status_room_new.reading_s2_status = new_status;
    
    // Update the MQTT state for reading S2
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S2_READ");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    flag_syn_status_room = true;
}

void handle_event_ceiling_s2(bool new_status)
{
    set_scene_status(new_status, scene_ceiling_s2_on, scene_ceiling_s2_off);
    status_room_new.ceiling_light_s2_status = new_status;
    
    // Update the MQTT state for ceiling S2
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S2_CEILING");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_night_s2(bool new_status)
{
    set_scene_status(new_status, scene_night_light_on, scene_night_light_off);
    status_room_new.night_light_s2_status = new_status;
    status_room_new.night_light_s3_status = new_status;

    // Update the MQTT state for night light S2-NIGHT
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S2_NIGHT");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S3-NIGHT
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S3_NIGHT");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_night_s3(bool new_status)
{
    set_scene_status(new_status, scene_night_light_on, scene_night_light_off);
    status_room_new.night_light_s3_status = new_status;
    status_room_new.night_light_s2_status = new_status;

    // Update the MQTT state for night light S2-NIGHT
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S2_NIGHT");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S3-NIGHT
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S3_NIGHT");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_master_m3(bool new_status)
{
    set_scene_status(new_status, scene_master_m3_on, scene_master_m3_off);
    status_room_new.master_m3_status = new_status;
    status_room_new.master_m2_status = new_status;
    status_room_new.master_m1_status = new_status;
    
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S1_M1");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S2_M2
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S2_M2");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    // S3_M3
    memset(&state_inout, 0, sizeof(state_in_out_t));
    strcpy(state_inout.key, "S3_M3");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void handle_event_ceiling_s3(bool new_status)
{
    set_scene_status(new_status, scene_ceiling_s3_on, scene_ceiling_s3_off);
    status_room_new.ceiling_light_s3_status = new_status;
    
    // Update the MQTT state for ceiling S3
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S3_CEILING");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    flag_syn_status_room = true;
}

void handle_event_reading_s3(bool new_status)
{
    set_scene_status(new_status, scene_reading_s3_on, scene_reading_s3_off);
    status_room_new.reading_s3_status = new_status;
    
    // Update the MQTT state for reading S3
    state_in_out_t state_inout = {0};
    strcpy(state_inout.key, "S3_READ");
    strcpy(state_inout.value, new_status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);

    flag_syn_status_room = true;
}

void scene_on_all()
{
    // on all
    bool on_status = true;

    handle_event_master_m1(on_status);
    handle_event_toilet(on_status);
    handle_event_wc_light(on_status);
    handle_event_minibar(on_status);
    handle_event_reading_s2(on_status);
    handle_event_ceiling_s2(on_status);
    handle_event_night_s2(on_status);
    handle_event_ceiling_s3(on_status);
    handle_event_reading_s3(on_status);
    status_room_new.master_m1_status = on_status;
    status_room_new.master_m2_status = on_status;
    status_room_new.master_m3_status = on_status;
    status_room_new.toilet_status = on_status;
    status_room_new.wc_light_status = on_status;
    status_room_new.minibar_status = on_status;
    status_room_new.reading_s2_status = on_status;
    status_room_new.ceiling_light_s2_status = on_status;
    status_room_new.night_light_s2_status = on_status;
    status_room_new.night_light_s3_status = on_status;
    status_room_new.ceiling_light_s3_status = on_status;
    status_room_new.reading_s3_status = on_status;
    flag_syn_status_room = true;
}

void scene_off_all()
{
    // off all
    bool off_status = false;
    handle_event_master_m1(off_status);
    handle_event_toilet(off_status);
    handle_event_wc_light(off_status);
    handle_event_minibar(off_status);
    handle_event_reading_s2(off_status);
    handle_event_ceiling_s2(off_status);
    handle_event_night_s2(off_status);
    handle_event_ceiling_s3(off_status);
    handle_event_reading_s3(off_status);
    status_room_new.master_m1_status = off_status;
    status_room_new.master_m2_status = off_status;
    status_room_new.master_m3_status = off_status;
    status_room_new.toilet_status = off_status;
    status_room_new.wc_light_status = off_status;
    status_room_new.minibar_status = off_status;
    status_room_new.reading_s2_status = off_status;
    status_room_new.ceiling_light_s2_status = off_status;
    status_room_new.night_light_s2_status = off_status;
    status_room_new.night_light_s3_status = off_status;
    status_room_new.ceiling_light_s3_status = off_status;
    status_room_new.reading_s3_status = off_status;
    flag_syn_status_room = true;
}

void handle_scene_unrented()
{
    scene_off_all();
    scene_itc_off();
    scene_idu_off();
    scene_dnd(false);
    scene_mur(false);
    char *json_string =
        create_json_dynamic("ROOM_STATUS", "UNRENTED", TYPE_STRING);
    publish_data_mqtt(json_string);
    char *json_string1 =
        create_json_dynamic("WELCOME_STATUS", "false", TYPE_STRING);
    publish_data_mqtt(json_string1);
    ESP_LOGI(__FUNCTION__, "Unrented scene activated.");

    char *json_string2 =
        create_json_dynamic("SET_BACK_ACTIVE", "false", TYPE_STRING);
    publish_data_mqtt(json_string2);

    // Đặt lại flag_checkin_first để chuẩn bị cho lần check-in tiếp theo
    pms_room_status.flag_checkin_first = true;

    free(json_string);
    free(json_string1);
    free(json_string2);
}

void handle_scene_staff_mode()
{
    scene_on_all();
    scene_itc_off();
    scene_idu_off();
    char *json_string =
        create_json_dynamic("ROOM_STATUS", "STAFF", TYPE_STRING);
    publish_data_mqtt(json_string);
    ESP_LOGI(__FUNCTION__, "Staff scene activated.");
    free(json_string);
}

void handle_scene_welcome()
{
    scene_on_all();
    scene_itc_on();
    scene_idu_on();
    scene_dnd(false);
    scene_mur(false);
    char *json_string =
        create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
    publish_data_mqtt(json_string);
    ESP_LOGI(__FUNCTION__, "Welcome scene activated.");
    free(json_string);
}

void handle_scene_standby()
{    
    scene_off_all();
    scene_itc_off();
    scene_idu_off();
    char *json_string =
        create_json_dynamic("ROOM_STATUS", "STANDBY", TYPE_STRING);
    publish_data_mqtt(json_string);
    ESP_LOGI(__FUNCTION__, "Standby scene activated.");
    free(json_string);
}

void handle_scene_occupied()
{
    ESP_LOGI(__FUNCTION__, "Starting occupied scene activation...");
    scene_itc_on();
    scene_idu_on();
    char *json_string = create_json_dynamic("ROOM_STATUS", "OCC", TYPE_STRING);
    publish_data_mqtt(json_string);
    char *json_string1 =
        create_json_dynamic("SET_BACK_ACTIVE", "false", TYPE_STRING);
    publish_data_mqtt(json_string1);
    free(json_string);
    free(json_string1);

    ESP_LOGI(__FUNCTION__, "Occupied scene activated.");
}
void scene_bell()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_BELL, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    generate_led_command(LED_BELL, true, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    vTaskDelay(pdMS_TO_TICKS(TIME_ACTIVE_BELL));
    generate_relay_command(RELAY_BELL, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    generate_led_command(LED_BELL, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
}

void scene_mur(bool status)
{
    char data_send[MAX_DATA_SIZE] = {0};
    state_in_out_t state_inout = {0};
    generate_led_command(LED_MUR, status, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    generate_led_command(LED_MUR_OUT_DOOR, status, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    strcpy(state_inout.key, "S1_MUR");
    strcpy(state_inout.value, status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    if (status)
    {
        status_outdoor.status_dnd = false;
        generate_led_command(LED_DND, false, data_send);
        modbusWrite(data_send, MAX_DATA_SIZE);
        generate_led_command(LED_DND_OUT_DOOR, false, data_send);
        modbusWrite(data_send, MAX_DATA_SIZE);
        strcpy(state_inout.key, "S1_DND");
        strcpy(state_inout.value, status_outdoor.status_dnd ? "true" : "false");
        push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    }
}

void scene_dnd(bool status)
{
    char data_send[MAX_DATA_SIZE] = {0};
    state_in_out_t state_inout = {0};
    generate_led_command(LED_DND, status, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    generate_led_command(LED_DND_OUT_DOOR, status, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    strcpy(state_inout.key, "S1_DND");
    strcpy(state_inout.value, status ? "true" : "false");
    push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    if (status)
    {
        status_outdoor.status_mur = false;
        generate_led_command(LED_MUR, false, data_send);
        modbusWrite(data_send, MAX_DATA_SIZE);
        generate_led_command(LED_MUR_OUT_DOOR, false, data_send);
        modbusWrite(data_send, MAX_DATA_SIZE);
        strcpy(state_inout.key, "S1_MUR");
        strcpy(state_inout.value, status_outdoor.status_mur ? "true" : "false");
        push_state_inout(&state_inout, 100 / portTICK_PERIOD_MS);
    }
}

void push_state_inout_mqtt(int index, bool value)
{
    state_in_out_t data_inout = {0};
    switch (index)
    {
        case RELAY_C1:
            strcpy(data_inout.key, "C1");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;

        case RELAY_C2:
            strcpy(data_inout.key, "C2");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C3:
            strcpy(data_inout.key, "C3");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C4:
            strcpy(data_inout.key, "C4");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C5:
            strcpy(data_inout.key, "C5");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;

        case RELAY_C6:
            strcpy(data_inout.key, "C6");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C7:
            strcpy(data_inout.key, "C7");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C8:
            strcpy(data_inout.key, "C8");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C9:
            strcpy(data_inout.key, "C9");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C10:
            strcpy(data_inout.key, "C10");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C11:
            strcpy(data_inout.key, "C11");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_C12:
            strcpy(data_inout.key, "C12");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_ICT:
            strcpy(data_inout.key, "ICT");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        case RELAY_IDU:
            strcpy(data_inout.key, "IDU");
            strcpy(data_inout.value, value ? "true" : "false");
            push_state_inout(&data_inout, 100 / portTICK_PERIOD_MS);
            break;
        default:
            break;
    }
}
