#ifndef HANDLEOUTPUTQUEUE_H
#define HANDLEOUTPUTQUEUE_H

#include "Global.h"

#define CRC16_MODBUS 0xA001
#define INIT_CHECKSUM 0xFFFF
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define TIMEOUT_RESPONSE 10
#define SIZE_DATA_CONTROL 8
#define TICK_TO_WAIT_QUEUE 10
#define HANDLE_OUPUT_DELAY_TASK (5)
#define TIME_SCAN (1000)
#define MAX_PACKGE_PARSE (50)


typedef struct inforDataModbusSend
{
    uint8_t address;
    uint16_t reg;
    uint16_t value;
} inforDataModbusSend_t;

enum FIELDS_NAME
{
    SLAVE_ADDR = 0,
    FUNC = 1,
    REG_H = 2,
    REG_L = 3,
    DATA_H = 4,
    DATA_L = 5,
    CRC_H = 6,
    CRC_l = 7,
};
#ifdef __cplusplus
extern "C"
{
#endif

void handleOutput(void *arg);

void HandleOutputQueue();

uint8_t getByteFromVariable(uint32_t input, uint8_t location);

uint16_t calCRC16(uint8_t *buff, int len);

void fillCRC(uint8_t *buff, int len);

// void convertUint32ToCharArray(uint32_t inputValue, char *output);

void fillDataSlaveAddress(uint8_t state, uint16_t regIn, uint8_t locaton,
                    uint16_t *regOut);

#ifdef __cplusplus
}
#endif

#endif