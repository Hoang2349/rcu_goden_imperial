#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h "
#include "freertos/task.h"
#define app_cmd_rs485_TXD (GPIO_NUM_17)
#define app_cmd_rs485_RXD (GPIO_NUM_16)
#define app_cmd_rs485_RTS (GPIO_NUM_13)
#define app_cmd_rs485_CTS (UART_PIN_NO_CHANGE)
#define app_cmd_rs485_BUF_SIZE (127)
#define app_cmd_rs485_BAUD_RATE 9600
#define PACKET_READ_TICS (100 / portTICK_RATE_MS)
#define OTA_TASK_STACK_SIZE (8192)
#define ECHO_TASK_PRIO (4)
#define app_cmd_rs485_PORT (UART_NUM_2)
#define ECHO_READ_TOUT (3)
#define INIT_CHECKSUM 0xFFFF
#define CRC16_MODBUS 0xA001
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define HI_UNIT32(a) (((a) >> 16) & 0xFFFF)
#define LO_UINT32(a) ((a) & 0xFFFF)
#define MAX_BTN (16)
#define MAX_QUEUE_PACKET_RS485 (100)
#define DEFAULT_RETURN_PINDEVICE (99)
#define DATA_PACKET_SIZE (8)
#define MIN_LEN_BUFFER (6)

#define MODBUS_ADDRESS_RCU_DEFAULT  (0x09)
#define REGISTER1_BUTTON_DEFAULT (0X0001)
#define REGISTER2_BUTTON_DEFAULT (0X0002)
#define SWITCH_RJ45_RCU_DEFAULT  (0x0000)

#define READ_INPUT_REGISTER (0X04)
#define READ_HOLDING_REGISTER (0X03)

#define BIT_COUNT_UINT16 (16)
#define PIN_START_REGISTER2 (15)
#define TIMEOUT_MODBUS (10000)

extern QueueHandle_t queuePacketRS485;
extern SemaphoreHandle_t xMutexModbusPort;
extern SemaphoreHandle_t xMutexStatusStm32;
extern bool flagStatusStm32;
extern bool flagModbusEmpty;
enum STATE_QUEUE
{
    NEED_TO_SEND,
    EMPTY,
};

typedef struct Modbus
{
    uint8_t address;
    uint8_t function;
    uint16_t registermb;
    uint16_t dataRegister;
    uint16_t crc16;
} mbSwitchFormat_t;

typedef struct
{
    uint8_t data[DATA_PACKET_SIZE];
    uint8_t size;
    uint32_t timeout;
} queuePacketOutRS485_t;

typedef struct rj45Register 
{
    uint16_t curState;
    uint16_t newState;
}rj45Register_t;


enum buttonRs485
{
    BUTTON_1 = 0x0001,
    BUTTON_2 = 0x0002,
    BUTTON_3 = 0x0004,
    BUTTON_4 = 0x0008,
    BUTTON_5 = 0x0010,
    BUTTON_6 = 0x0020,
    BUTTON_7 = 0x0040,
    BUTTON_8 = 0x0080,
    BUTTON_9 = 0x0100,
    BUTTON_10 = 0x0200,
    BUTTON_11 = 0x0400,
    BUTTON_12 = 0x0800,
    BUTTON_13 = 0x1000,
    BUTTON_14 = 0x2000,
    BUTTON_15 = 0x4000,
    BUTTON_16 = 0x8000,
};

static uint16_t mapButton[2][MAX_BTN] = {
    {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7,
     BUTTON_8, BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12, BUTTON_13, BUTTON_14,
     BUTTON_15, BUTTON_16},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};

#ifdef __cplusplus
extern "C"
{
#endif

    void initRs485();
    void handleRs485(void *arg);
    void handleMessageRS485(uint8_t *rxMessage, int len);
    void sendDataRs485(queuePacketOutRS485_t *dataQueueTmp);
    uint8_t sendDataModbus(uint8_t *data, uint8_t size, uint32_t timeout);
    bool compareArray(const uint8_t *input1, const uint8_t *input2, int len);
    void copyArray(uint8_t *des, uint8_t *src, uint16_t pStartReadSrc,
                   uint16_t len);
    bool rs485Reciver();
    uint32_t parseDataSwitchModbus(uint8_t addr, uint16_t reg, uint16_t val);
    uint32_t parseDataSwitchRj45(uint8_t addr, uint16_t reg, uint16_t val);
    uint8_t getPinSwitchModbus(uint16_t value);
    uint8_t getPinSwitchRJ45(uint16_t value);
    bool checkMessageModbus(uint8_t *rxMessage, int len);
    void handleMessageRS485(uint8_t *rxMessage, int len);
    int crc16(uint8_t *buff, int len);
    void modbusInit();

    void modbusWrite(char *data, int len);
    bool modbusRead(char *data);
    void checkLineModbus(bool status);
#ifdef __cplusplus
}
#endif
#endif