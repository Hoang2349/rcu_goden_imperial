#ifndef __APP_CONTROL_RELAY_H__
#define __APP_CONTROL_RELAY_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_DATA_SIZE 16
enum RelayIndex
{
    RELAY_IDU = 0x0F,
    RELAY_C1 = 0x0E,
    RELAY_C2 = 0x0D,
    RELAY_C3 = 0x0C,
    RELAY_C4 = 0x0B,
    RELAY_C5 = 0x0A,
    RELAY_C6 = 0x09,
    RELAY_C7 = 0x08,
    RELAY_C8 = 0x07,
    RELAY_C9 = 0x06,
    RELAY_C10 = 0x05,
    RELAY_C11 = 0x04,
    RELAY_C12 = 0x03,
    RELAY_ICT = 0x02,
    RELAY_BELL = 0x01,
};

enum Ledindex
{
    LED_BELL = 0x01,
    LED_DND_OUT_DOOR = 0x02,
    LED_MUR_OUT_DOOR = 0x03,
    LED_DND = 0x06,
    LED_MUR = 0x05,
    LED_MASTER_M1 = 0x04,
    LED_TOILET = 0x0A,
    LED_BATHROOM = 0x0B,
    LED_READING_S2 = 0x11,
    LED_DECORATION_S2 = 0x13,
    LED_MASTER_M2 = 0x10,
    LED_MASTER_M3 = 0x16,
    LED_MINIBAR = 0x0D,
    LED_SPORT_LIGHT = 0x12,
    LED_DECORATION_S3 = 0x14,
    LED_COVER_LIGHT = 0x17,
    LED_READING_S3 = 0x18,
};




void scene_goden_init(void);
void scene_goden_deinit(void);
void scene_master_m1_on();
void scene_master_m1_off();
void scene_master_m2_on();
void scene_master_m2_off();
void scene_master_m3_on();
void scene_master_m3_off();
void scene_toilet_on();
void scene_toilet_off();
void scene_wc_light_on();
void scene_wc_light_off();
void scene_reading_s3_on();
void scene_reading_s3_off();
void scene_ceiling_s2_on();
void scene_ceiling_s2_off();
void scene_night_light_on();
void scene_night_light_off();
void scene_ceiling_s3_on();
void scene_ceiling_s3_off();
void scene_minibar_on();
void scene_minibar_off();
void scene_bell_on();
void scene_bell_off();
void scene_itc_on();
void scene_itc_off();
void scene_idu_on();
void scene_idu_off();
void handle_event_master_m1(bool new_status);
void handle_event_master_m2(bool new_status);
void handle_event_toilet(bool new_status);
void handle_event_wc_light(bool new_status);
void handle_event_minibar(bool new_status);
void handle_event_reading_s2(bool new_status);
void handle_event_ceiling_s2(bool new_status);
void handle_event_night_s2(bool new_status);
void handle_event_night_s3(bool new_status);
void handle_event_master_m3(bool new_status);
void handle_event_ceiling_s3(bool new_status);
void handle_event_reading_s3(bool new_status);

void handle_scene_unrented();
void handle_scene_staff_mode();
void handle_scene_welcome();
void handle_scene_standby();
void handle_scene_occupied();

void scene_bell();
void scene_dnd(bool status);
void scene_mur(bool status);

void push_state_inout_mqtt(int index, bool value);


#endif //  __APP_CONTROL_RELAY_H__