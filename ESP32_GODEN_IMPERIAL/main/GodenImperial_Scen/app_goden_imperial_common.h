#ifndef APP_GODEN_IMPERIAL_COMMON_H
#define APP_GODEN_IMPERIAL_COMMON_H

#include <stdint.h>
#include <stdbool.h>




#define MAX_RELAY_CONTROL 12
#define MAX_LED_CONTROL 12
typedef struct status_room
{
    bool master_m1_status;
    bool toilet_status;
    bool wc_light_status;
    bool minibar_status;
    bool master_m2_status;
    bool reading_s2_status;
    bool ceiling_light_s2_status;
    bool night_light_s2_status;
    bool night_light_s3_status;
    bool master_m3_status;
    bool ceiling_light_s3_status;
    bool reading_s3_status;
} status_room_t;

typedef struct status_outdoor
{
    bool status_dnd;
    bool status_mur;
    bool status_bell;
} status_outdoor_t;
typedef struct
{
    bool status_room;
    bool flag_checkin_first;
    bool flag_checkout_first;
    bool flag_setback;
    bool status_human;
} pms_room_status_t;

typedef enum
{
    //standby/unrented/occ/unocc/staff/ofs
    STANDBY = 0,
    UNRENTED = 1,
    OCC = 2,
    UNOCC = 3,
    STAFF = 4,
    OFS = 5,
} status_mode_t;

typedef struct
{
    bool flag_door_sensor;
    bool new_status_door_sensor;
    bool flag_motion_sensor;
} status_sensor_t;

typedef struct
{
    uint8_t index;   
    uint8_t value;  
} output_state_t;

enum ROOM_STATUS
{
    ROOM_UNRENT = 0,
    ROOM_RENTED,
};

enum STATUS_HUMAN_ROOM
{
    UNOCCUPIED = 0,
    OCCUPIED = 1,
};

enum 
{
    INACTIVE = 0,
    ACTIVE = 1,
};

enum 
{
    DOOR_CLOSE = 0,
    DOOR_OPEN = 1,
};


// extern status_room_t status_room;
// extern uint16_t version;

extern status_room_t status_room_cur;
extern status_room_t status_room_new;
extern bool flag_syn_status_room;
extern pms_room_status_t pms_room_status;
extern status_sensor_t status_sensor;
extern status_outdoor_t status_outdoor;
extern uint32_t time_crossing_set;
extern double time_door_ajar;
#endif // APP_GODEN_IMPERIAL_COMMON_H