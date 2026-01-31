#ifndef APP_HANDLE_INPUT_H
#define APP_HANDLE_INPUT_H

#include <stdint.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
enum input_event
{
    EVENT_BELL = 0x01,
    // EVENT_2 = 0x02,
    // EVENT_3 = 0x03,

    EVENT_DND = 0x04,
    EVENT_MUR = 0x05,
    EVENT_MASTER_M1 = 0x06,

    EVENT_DOOR = 0x07,
    EVENT_MOTION = 0x08,
    EVENT_DOOR_CLOSE = 0x09,

    EVENT_TOILET = 0x0A,
    EVENT_BATHROOM = 0x0B,
    EVENT_12 = 0x0C,

    EVENT_MINIBAR = 0x0D,
    // EVENT_14 = 0x0E,
    // EVENT_15 = 0x0F,

    EVENT_MASTER_M2 = 0x10,
    EVENT_READING_S2 = 0x11,
    EVENT_SPORT_LIGHT = 0x12,

    EVENT_DECORATION_S2 = 0x13,
    EVENT_DECORATION_S3 = 0x14,
    // EVENT_21 = 0x15,

    EVENT_MASTER_M3 = 0x16,
    EVENT_COVER_LIGHT = 0x17,
    EVENT_READING_S3 = 0x18,

    // EVENT_25 = 0x19,
    // EVENT_26 = 0x1A,
    // EVENT_27 = 0x1B,
};
enum queueStautus
{
    QUEUE_FAIL = 0,
    QUEUE_OK = 1,
};

#define MAX_QUEUE_INPUT_STM32 10
#define MAX_LENGTH_INPUT_STM32 8
#define LENGTH_INPUT_STM32 8
#define TICK_TO_WAIT_QUEUE 10

#ifdef __cplusplus
extern "C"
{
#endif
    void queueInputStm32Init();
    uint8_t queueInputStm32Push(const uint8_t *input);
    uint8_t queueInputStm32Pop(uint8_t *output);
    void check_active_scen(uint8_t pin_active, uint8_t status);
    void init_goden_imperial_input();

#ifdef __cplusplus
}
#endif

#endif  // APP_HANDLE_INPUT_H