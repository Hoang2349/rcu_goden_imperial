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
    update_setback_status_led(output[9], output[10]);
}

void generate_relay_command(uint8_t relay_index, bool turn_on, char *output)
{
    const char template_data[MAX_DATA_SIZE] = {
        0x09, 0x10, 0x00, 0x01, 0x04, 0x08, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(output, template_data, MAX_DATA_SIZE);

    output[7] = relay_index;             // Relay number
    output[10] = turn_on ? 0xFF : 0x00;  // On/off value
    update_setback_status_relay(relay_index, output[10]);
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
}
void scene_reading_s2_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C6, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C6, false);
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
}
void scene_reading_s3_off()
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_relay_command(RELAY_C8, false, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    push_state_inout_mqtt(RELAY_C8, false);
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
    flag_syn_status_room = true;
}

void handle_event_master_m2(bool new_status)
{
    set_scene_status(new_status, scene_master_m2_on, scene_master_m2_off);
    status_room_new.master_m1_status = new_status;
    status_room_new.master_m2_status = new_status;
    status_room_new.master_m3_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_toilet(bool new_status)
{
    set_scene_status(new_status, scene_toilet_on, scene_toilet_off);
    status_room_new.toilet_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_wc_light(bool new_status)
{
    set_scene_status(new_status, scene_wc_light_on, scene_wc_light_off);
    status_room_new.wc_light_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_minibar(bool new_status)
{
    set_scene_status(new_status, scene_minibar_on, scene_minibar_off);
    status_room_new.minibar_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_reading_s2(bool new_status)
{
    set_scene_status(new_status, scene_reading_s2_on, scene_reading_s2_off);
    status_room_new.reading_s2_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_ceiling_s2(bool new_status)
{
    set_scene_status(new_status, scene_ceiling_s2_on, scene_ceiling_s2_off);
    status_room_new.ceiling_light_s2_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_night_s2(bool new_status)
{
    set_scene_status(new_status, scene_night_light_on, scene_night_light_off);
    status_room_new.night_light_s2_status = new_status;
    status_room_new.night_light_s3_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_night_s3(bool new_status)
{
    set_scene_status(new_status, scene_night_light_on, scene_night_light_off);
    status_room_new.night_light_s3_status = new_status;
    status_room_new.night_light_s2_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_master_m3(bool new_status)
{
    set_scene_status(new_status, scene_master_m3_on, scene_master_m3_off);
    status_room_new.master_m3_status = new_status;
    status_room_new.master_m2_status = new_status;
    status_room_new.master_m1_status = new_status;
    flag_syn_status_room = true;
}

void handle_event_ceiling_s3(bool new_status)
{
    set_scene_status(new_status, scene_ceiling_s3_on, scene_ceiling_s3_off);
    flag_syn_status_room = true;
    status_room_new.ceiling_light_s3_status = new_status;
}

void handle_event_reading_s3(bool new_status)
{
    set_scene_status(new_status, scene_reading_s3_on, scene_reading_s3_off);
    status_room_new.reading_s3_status = new_status;
    flag_syn_status_room = true;
}

void scene_on_all()
{
    //on all
    bool on_status = true;
    scene_master_m1_on();
    scene_toilet_on();
    scene_wc_light_on();
    scene_minibar_on();
    scene_reading_s2_on();
    scene_ceiling_s2_on();
    scene_night_light_on();
    scene_ceiling_s3_on();
    scene_reading_s3_on();
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
    //off all
    bool off_status = false;
    scene_master_m1_off();
    scene_toilet_off();
    scene_wc_light_off();
    scene_minibar_off();
    scene_reading_s2_off();
    scene_ceiling_s2_off();
    scene_night_light_off();
    scene_ceiling_s3_off();
    scene_reading_s3_off();
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

void handle_scene_unrentd()
{
    scene_off_all();
    scene_itc_off();
    scene_idu_off();
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
    char *json_string =
        create_json_dynamic("ROOM_STATUS", "WELCOME", TYPE_STRING);
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
    handle_scene_setback();
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

void handle_scene_setback()
{
    for (int i = 0; i < MAX_RELAY_CONTROL; i++)
    {
        char data_send[MAX_DATA_SIZE] = {0};
        generate_relay_command(state_setback.relay[i].index,
                               state_setback.relay[i].value, data_send);
        modbusWrite(data_send, MAX_DATA_SIZE);
        push_state_inout_mqtt(state_setback.relay[i].index,
                              state_setback.relay[i].value);
    }

    status_room_new.master_m1_status =
        state_setback.status_room.master_m1_status;
    status_room_new.toilet_status = state_setback.status_room.toilet_status;
    status_room_new.wc_light_status = state_setback.status_room.wc_light_status;
    status_room_new.minibar_status = state_setback.status_room.minibar_status;
    status_room_new.master_m2_status =
        state_setback.status_room.master_m2_status;
    status_room_new.reading_s2_status =
        state_setback.status_room.reading_s2_status;
    status_room_new.ceiling_light_s2_status =
        state_setback.status_room.ceiling_light_s2_status;
    status_room_new.night_light_s2_status =
        state_setback.status_room.night_light_s2_status;
    status_room_new.night_light_s3_status =
        state_setback.status_room.night_light_s3_status;
    status_room_new.master_m3_status =
        state_setback.status_room.master_m3_status;
    status_room_new.ceiling_light_s3_status =
        state_setback.status_room.ceiling_light_s3_status;
    status_room_new.reading_s3_status =
        state_setback.status_room.reading_s3_status;
    flag_syn_status_room = true;
}
//===============led indicate===================//

void hanlde_led_switch(uint8_t led_index, bool status)
{
    char data_send[MAX_DATA_SIZE] = {0};
    generate_led_command(led_index, status, data_send);
    modbusWrite(data_send, MAX_DATA_SIZE);
    ESP_LOGI(__FUNCTION__, "LED %d %s", led_index, status ? "ON" : "OFF");
}

void handle_led_status()
{
    HANDLE_LED_STATUS(master_m1_status, hanlde_led_switch, LED_MASTER_M1);
    HANDLE_LED_STATUS(master_m2_status, hanlde_led_switch, LED_MASTER_M2);
    HANDLE_LED_STATUS(toilet_status, hanlde_led_switch, LED_TOILET);
    HANDLE_LED_STATUS(wc_light_status, hanlde_led_switch, LED_BATHROOM);
    HANDLE_LED_STATUS(minibar_status, hanlde_led_switch, LED_MINIBAR);
    HANDLE_LED_STATUS(master_m2_status, hanlde_led_switch, LED_MASTER_M2);
    HANDLE_LED_STATUS(reading_s2_status, hanlde_led_switch, LED_READING_S2);
    HANDLE_LED_STATUS(ceiling_light_s2_status, hanlde_led_switch, LED_SPORT_LIGHT);
    HANDLE_LED_STATUS(night_light_s2_status, hanlde_led_switch,
                      LED_DECORATION_S2);
    HANDLE_LED_STATUS(night_light_s3_status, hanlde_led_switch,
                      LED_DECORATION_S3);
    HANDLE_LED_STATUS(master_m3_status, hanlde_led_switch, LED_MASTER_M3);
    HANDLE_LED_STATUS(ceiling_light_s3_status, hanlde_led_switch, LED_COVER_LIGHT);
    HANDLE_LED_STATUS(reading_s3_status, hanlde_led_switch, LED_READING_S3);
}

// handle upadte status setback
void update_setback_status_relay(uint8_t index, uint8_t value)
{
    if ((pms_room_status.status_human == OCCUPIED))
    {
        for (int i = 0; i < MAX_RELAY_CONTROL; i++)
        {
            if (state_setback.relay[i].index == index)
            {
                state_setback.relay[i].value = value;
                ESP_LOGI(__FUNCTION__, "Relay %d status updated to %d", index,
                         value);
                break;
            }
        }
    }
}

void update_setback_status_led(uint8_t index, uint8_t value)
{
    if ((pms_room_status.status_human == OCCUPIED))
    {
        for (int i = 0; i < MAX_LED_CONTROL; i++)
        {
            if (state_setback.led[i].index == index)
            {
                state_setback.led[i].value = value;
                ESP_LOGI(__FUNCTION__, "LED %d status updated to %d", index,
                         value);
                break;
            }
        }
    }
}

void update_setback_status_room()
{
    if ((pms_room_status.status_human == OCCUPIED))
    {
        state_setback.status_room.master_m1_status =
            status_room_cur.master_m1_status;
        state_setback.status_room.toilet_status = status_room_cur.toilet_status;
        state_setback.status_room.wc_light_status =
            status_room_cur.wc_light_status;
        state_setback.status_room.minibar_status =
            status_room_cur.minibar_status;
        state_setback.status_room.master_m2_status =
            status_room_cur.master_m2_status;
        state_setback.status_room.reading_s2_status =
            status_room_cur.reading_s2_status;
        state_setback.status_room.ceiling_light_s2_status =
            status_room_cur.ceiling_light_s2_status;
        state_setback.status_room.night_light_s2_status =
            status_room_cur.night_light_s2_status;
        state_setback.status_room.night_light_s3_status =
            status_room_cur.night_light_s3_status;
        state_setback.status_room.master_m3_status =
            status_room_cur.master_m3_status;
        state_setback.status_room.ceiling_light_s3_status =
            status_room_cur.ceiling_light_s3_status;
        state_setback.status_room.reading_s3_status =
            status_room_cur.reading_s3_status;
    }
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

void scene_mur(uint8_t status)
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

void scene_dnd(uint8_t status)
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
