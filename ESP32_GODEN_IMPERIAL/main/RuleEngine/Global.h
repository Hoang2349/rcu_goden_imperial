#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdint.h>

#include <vector>
#include "MapRuleMqtt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h "
#include "freertos/task.h"
#define DEBUG_LOGE ESP_LOGE

typedef struct
{
    uint8_t type;
    uint32_t address;
    uint8_t value;
} inputItem_t;

typedef struct
{
    uint8_t type;
    uint32_t address;
    uint8_t value;
} outputItem_t;

typedef struct
{
    inputItem_t input;
    std::vector<outputItem_t> output;
} rule_t;

enum CheckError
{
    RULE_ERROR = 0,
    RULE_OK = 1,
};

enum TypeValue
{
    RULE_OFF = 0,
    RULE_ON = 1,
    RULE_TOGGLE = 2,
};

enum MB_FC
{
    MB_FC_NONE = 0,       /*!< null operator */
    MB_FC_READ_COILS = 1, /*!< FCT=1 -> read coils or digital outputs */
    MB_FC_READ_DISCRETE_INPUT = 2, /*!< FCT=2 -> read digital inputs */
    MB_FC_READ_REGISTERS = 3, /*!< FCT=3 -> read registers or analog outputs */
    MB_FC_READ_INPUT_REGISTER = 4, /*!< FCT=4 -> read analog inputs */
    MB_FC_WRITE_COIL = 5,          /*!< FCT=5 -> write single coil or output */
    MB_FC_WRITE_REGISTER = 6,      /*!< FCT=6 -> write single register */
    MB_FC_WRITE_MULTIPLE_COILS =
        15, /*!< FCT=15 -> write multiple coils or outputs */
    MB_FC_WRITE_MULTIPLE_REGISTERS =
        16 /*!< FCT=16 -> write multiple registers */
};

#endif