#include "Modbus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Global.h"
#include "QueueInput.h"
#include "app_handle_input.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "sdkconfig.h"

SemaphoreHandle_t xMutexModbusPort = NULL;
SemaphoreHandle_t xMutexStatusStm32 = NULL;

bool flagModbusEmpty = false;
bool flagStatusStm32 = false;

QueueHandle_t queuePacketRS485;
unsigned int errorPacketSend = 0;
queuePacketOutRS485_t packetLastSend;

/**
 * @brief Init RS485 Port and config for RS485
 */
void initRs485()
{
    esp_log_level_set(__FUNCTION__, ESP_LOG_INFO);
    ESP_LOGI(__FUNCTION__, "initRs485 Start\r\n");

    // const int uartNum = app_cmd_rs485_PORT;
    uart_config_t uartSetConfig = {
        .baud_rate = app_cmd_rs485_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(
        app_cmd_rs485_PORT, app_cmd_rs485_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(app_cmd_rs485_PORT, &uartSetConfig));
    ESP_LOGI(__FUNCTION__, "UART set pins, mode and install driver.");
    ESP_ERROR_CHECK(uart_set_pin(app_cmd_rs485_PORT, app_cmd_rs485_TXD,
                                 app_cmd_rs485_RXD, app_cmd_rs485_RTS,
                                 app_cmd_rs485_CTS));
    // Set RS485 half duplex mode
    ESP_ERROR_CHECK(
        uart_set_mode(app_cmd_rs485_PORT, UART_MODE_RS485_HALF_DUPLEX));
    ESP_ERROR_CHECK(uart_set_rx_timeout(app_cmd_rs485_PORT, ECHO_READ_TOUT));

    ESP_LOGI(__FUNCTION__, "initRs485 End\r\n");
}

/**
 * @brief Function handle transfer/recive data via RS485
 */
void handleRs485(void *arg)
{
    while (1)
    {
        if (xSemaphoreTake(xMutexModbusPort, (TickType_t)100) == pdTRUE)
        {
            queuePacketOutRS485_t dataQueueTmp;
            if (xQueueReceive(queuePacketRS485, &dataQueueTmp,
                              (TickType_t)TICK_TO_WAIT_QUEUE) == pdPASS)
            {
                sendDataRs485(&dataQueueTmp);
            }
            checkLineModbus(rs485Reciver());
            // free key
            xSemaphoreGive(xMutexModbusPort);
        }
        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

/**
 * @brief Function handle transfer/recive data via RS485
 * @param queuePacketOutRS485_t *dataQueueTmp: struct includes the data need to
 * send
 * @return true or false
 */
void sendDataRs485(queuePacketOutRS485_t *dataQueueTmp)
{
    //-------------------------debug data truyen rs485-----------------//
    ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, dataQueueTmp->data,
                             dataQueueTmp->size, ESP_LOG_INFO);

    // compare the last packet that was sent with the packet now.
    if (compareArray(&packetLastSend.data[0], &dataQueueTmp->data[0],
                     dataQueueTmp->size) == true)
    {
        ESP_LOGE(__FUNCTION__, "last packet = present packet -> not sent");
        return;
    }
    else
    {
        uart_write_bytes(app_cmd_rs485_PORT, (char *)&dataQueueTmp->data,
                         dataQueueTmp->size);

        // update packetLastSend - start
        copyArray(&packetLastSend.data[0], &dataQueueTmp->data[0], 0,
                  dataQueueTmp->size);
        packetLastSend.size = dataQueueTmp->size;
        // update packetLastSend - end

        uint8_t dataReceive[dataQueueTmp->size];
        int len =
            uart_read_bytes(app_cmd_rs485_PORT, dataReceive, dataQueueTmp->size,
                            dataQueueTmp->timeout / portTICK_RATE_MS);
        if (len <= 0)
        {
            errorPacketSend++;
            ESP_LOGE(__FUNCTION__, "line = %d, errorPacketSend++ = %d\r\n",
                     __LINE__, errorPacketSend);
            // todo note log
            return;
        }

        if (!compareArray(dataReceive, &dataQueueTmp->data[0],
                          dataQueueTmp->size))
        {
            errorPacketSend++;
            ESP_LOGE(__FUNCTION__, "line = %d, errorPacketSend++ = %d\r\n",
                     __LINE__, errorPacketSend);
            // todo note log
            return;
        }
    }

    return;  // success
}

/**
 * @brief Function handle transfer/recive data via RS485
 * @param uint8_t *data: Pointer data location start
 * @param uint8_t size: size of data send
 * @param uint8_t timeout: time respond from slave
 * @return true or false
 */
uint8_t sendDataModbus(uint8_t *data, uint8_t size, uint32_t timeout)
{
    // ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, data, size, ESP_LOG_INFO);

    queuePacketOutRS485_t dataQueueTmp;
    copyArray(&dataQueueTmp.data[0], data, 0, size);
    dataQueueTmp.size = size;
    dataQueueTmp.timeout = timeout;

    if (xQueueSendToBack(queuePacketRS485, (void *)&dataQueueTmp,
                         (TickType_t)TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        DEBUG_LOGE(__FUNCTION__, "Error xQueueSendToBack = errQUEUE_FULL");
        errorPacketSend++;
        DEBUG_LOGE(__FUNCTION__, "line = %d, errorPacketSend++ = %d\r\n",
                   __LINE__, errorPacketSend);
        return false;
    }
    return true;
}

/**
 * @brief Function compare data array
 * @param const uint8_t *input1: pointer address start array 1
 * @param const uint8_t *input2: pointer address start array 2
 * @return true or false
 */
bool compareArray(const uint8_t *input1, const uint8_t *input2, int len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        if (*input1 != *input2)
        {
            return false;
        }
        input1++;
        input2++;
    }
    return true;
}

/**
 * @brief coppy array
 * @param uint8_t *des: destination array
 * @param uint8_t *src: source array
 * @param uint16_t pStartReadSrc: location start coppy
 */
void copyArray(uint8_t *des, uint8_t *src, uint16_t pStartReadSrc, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        *des = *(src + pStartReadSrc);
        des++;
        src++;
    }
}

/**
 * @brief Function reciver data on rs485 and check CRC data
 * @return true or false
 */
bool rs485Reciver()
{
    static uint8_t data[app_cmd_rs485_BUF_SIZE] = {0};
    int len = uart_read_bytes(app_cmd_rs485_PORT, data, app_cmd_rs485_BUF_SIZE,
                              10 / portTICK_RATE_MS);
    if (len > 0)
    {
        ESP_LOGW(__FUNCTION__, "=============================================");
        ESP_LOGW(__FUNCTION__, "Received %u bytes:", len);
        ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, data, len, ESP_LOG_INFO);

        if (checkMessageModbus(data, len) == false)
        {
            ESP_LOGE(__FUNCTION__, "FAIL");
            return false;
        }
        else
        {
            ESP_LOGI(__FUNCTION__, "\033[0;34m PASS \033[0m");
            // handleMessageRS485(data, len);
            if (len == LENGTH_INPUT_STM32)
            {
                queueInputStm32Push(data);
            }
        }
        ESP_LOGW(__FUNCTION__, "=============================================");
        return true;
    }
    return false;
}

/**
 * @brief handle check CRC 16
 * @return true or false
 */
bool checkMessageModbus(uint8_t *rxMessage, int len)
{
    static int crc = 0;
    if (len < MIN_LEN_BUFFER)
    {
        return false;
    }
    crc = crc16(rxMessage, len - 2);
    ESP_LOGI(__FUNCTION__, "CRC: 0x%X", (uint16_t)crc);
    //--------------------------RS485---------------------------//
    if (LO_UINT16(crc) == rxMessage[len - 2] &&
        HI_UINT16(crc) == rxMessage[len - 1] &&
        (rxMessage[1] == READ_HOLDING_REGISTER ||
         rxMessage[1] == READ_INPUT_REGISTER))
    {
        return true;
    }
    return false;
}

/**
 * @brief hàm xử lí các gói tin nhận được từ mobbus xử lí gói tin và đưa vào
 * queueInput chứa các gia trị input
 * @param  uint8_t *rxMessage gói tin nhận được từ cổng RS485
 * @paragraph
 *    0   |   1    | 2   |   3    |   4    |   5    |    6 |   7   |  8    |
 *  Addr    Func    Len   data1-H  data1-L    ...     ...     CRC-L   CRC-H
 *
 * @param int len độ dài gói tin
 * @return đẩy dữu liệu dạng VariableInput_t vào queueInput
 */
void handleMessageRS485(uint8_t *rxMessage, int len)
{
    static rj45Register_t rj45Input1 = {0};
    static rj45Register_t rj45Input2 = {0};
    inputItem_t dataPush = {0};
    mbSwitchFormat_t ptr = {0};
    // controll handle input rj45
    if (rxMessage[0] == MODBUS_ADDRESS_RCU_DEFAULT)  // default for RCU
    {
        // parse data Input for data in1
        rj45Input1.newState =
            ((uint16_t)rxMessage[3] << 8) | (uint16_t)rxMessage[4];
        // parse data Input for data in2
        rj45Input2.newState =
            ((uint16_t)rxMessage[5] << 8) | (uint16_t)rxMessage[6];
        for (int i = 0; i < BIT_COUNT_UINT16 - 1; i++)
        {
            if ((rj45Input1.curState & (0x0001 << i)) !=
                (rj45Input1.newState & (0x0001 << i)))
            {
                uint8_t value = 0;
                uint8_t location = i + 1;
                (rj45Input1.newState & (0x0001 << i)) == 0 ? (value = 0)
                                                           : (value = 1);
                dataPush.type = switchRj45;
                dataPush.address =
                    (uint32_t)(MODBUS_ADDRESS_RCU_DEFAULT << 24) |
                    (uint32_t)(REGISTER1_BUTTON_DEFAULT < 8) |
                    (uint32_t)(location);
                dataPush.value = value;
                queueInputPush(&dataPush);
                continue;
            }
            if ((rj45Input2.curState & (0x0001 << i)) !=
                (rj45Input2.newState & (0x0001 << i)))
            {
                uint8_t value = 0;
                uint8_t location = i + BIT_COUNT_UINT16;

                (rj45Input2.newState & (0x0001 << i)) == 0 ? (value = 0)
                                                           : (value = 1);
                dataPush.type = switchRj45;
                dataPush.address =
                    (uint32_t)(MODBUS_ADDRESS_RCU_DEFAULT << 24) |
                    (uint32_t)(REGISTER2_BUTTON_DEFAULT << 8) |
                    (uint32_t)(location);
                dataPush.value = value;
                queueInputPush(&dataPush);
            }
        }
        rj45Input1.curState = rj45Input1.newState;
        rj45Input2.curState = rj45Input2.newState;
    }
    // handle Button rs485
    else
    {
        ptr.address = rxMessage[0];
        ptr.registermb = (uint16_t)rxMessage[2] << 8 | (uint16_t)rxMessage[3];
        ptr.dataRegister = (uint16_t)rxMessage[4] << 8 | (uint16_t)rxMessage[5];
        dataPush.type = switchModbus;
        dataPush.address = parseDataSwitchModbus(ptr.address, ptr.registermb,
                                                 ptr.dataRegister);
        dataPush.value = 1;
        if (dataPush.address != 0)
        {
            // enqueue data to queue input
            queueInputPush(&dataPush);
        }
    }
}

/**
 * @brief function parse data switch Modbus after push to queue handle struct
 * data: 0xAABBCCDD
 * @param  uint32_t AA    |    BB   |   CC    |   DD   |
 * @param  discrip  addr  |  Reg-H  |  Reg-L  | Value  |
 * @param uint8_t addr: Address slave
 * @param uint16_t reg: Loction Register
 * @param uint16_t value: value register
 * @return variable 32bit struct address or 0(error)
 */
uint32_t parseDataSwitchModbus(uint8_t addr, uint16_t reg, uint16_t val)
{
    static uint32_t structAddress = 0;
    uint8_t value;
    value = getPinSwitchModbus(val);
    if (value == DEFAULT_RETURN_PINDEVICE)
    {
        return RULE_ERROR;
    }
    structAddress = (uint32_t)addr << 24 | (uint32_t)reg << 8 | (uint32_t)value;
    return structAddress;
}

/**
 * @brief function parse data switch Rj45 after push to queue handle struct
 * data: 0xAABBCCDD
 * @param  uint32_t AA    |    BB   |   CC    |   DD   |
 * @param  discrip  addr  |  Reg-H  |  Reg-L  | Value  |
 * @param uint8_t addr: Address slave
 * @param uint16_t reg: Loction Register
 * @param uint16_t value: value register
 * @return variable 32bit struct address or 0(error)
 */
uint32_t parseDataSwitchRj45(uint8_t addr, uint16_t reg, uint16_t val)
{
    static uint32_t structAddress = 0;
    uint8_t pin = DEFAULT_RETURN_PINDEVICE;
    if (reg == 1)
    {
        pin = getPinSwitchRJ45(val);
        if (pin == DEFAULT_RETURN_PINDEVICE)
        {
            return RULE_ERROR;
        }
        reg = SWITCH_RJ45_RCU_DEFAULT;
        structAddress =
            (uint32_t)addr << 24 | (uint32_t)reg << 8 | (uint32_t)pin;
        return structAddress;
    }
    if (reg == 2)
    {
        pin = getPinSwitchRJ45(val);
        if (pin == DEFAULT_RETURN_PINDEVICE)
        {
            return RULE_ERROR;
        }
        pin = pin + PIN_START_REGISTER2;
        reg = SWITCH_RJ45_RCU_DEFAULT;
        structAddress =
            (uint32_t)addr << 24 | (uint32_t)reg << 8 | (uint32_t)pin;
        return structAddress;
    }
    return RULE_ERROR;
}

/**
 * @brief Hàm xử lý lấy thông tin nút nhấn được nhấn
 * @param uint16_t value: Giá trị thanh ghi thay đổi
 * @return trả về giá trị nút nhấn vừa được tác động
 */
uint8_t getPinSwitchModbus(uint16_t value)
{
    value = value & 0x00FF;
    for (uint8_t i = 0; i < MAX_BTN; i++)
    {
        if (value == mapButton[0][i])
        {
            // printf("Pin: %d\n", mapButton[1][i]);
            return mapButton[1][i];
        }
    }
    return DEFAULT_RETURN_PINDEVICE;
}

/**
 * @brief Hàm xử lý lấy thông tin nút nhấn được nhấn
 * @param uint16_t value: Giá trị thanh ghi thay đổi
 * @return trả về giá trị nút nhấn vừa được tác động
 */
uint8_t getPinSwitchRJ45(uint16_t value)
{
    value = value & 0xFFFF;
    for (uint8_t i = 0; i < MAX_BTN; i++)
    {
        if (value == mapButton[0][i])
        {
            // printf("Pin: %d\n", mapButton[1][i]);
            return mapButton[1][i];
        }
    }
    return DEFAULT_RETURN_PINDEVICE;
}

/**
 * @brief Function send data to rs485 via UART
 * @param char *data: pointer data start address
 * @param int len: length of data need to send
 */
void modbusWrite(char *data, int len)
{
    uart_write_bytes(app_cmd_rs485_PORT, data, len);
    // ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, data, len, ESP_LOG_INFO);
}

/**
 * @brief Function read data recive to UART Port
 * @param char *data: point to address data store
 * @return true or false(no data)
 */
bool modbusRead(char *data)
{
    uart_flush(app_cmd_rs485_PORT);
    int len = uart_read_bytes(app_cmd_rs485_PORT, data, app_cmd_rs485_BUF_SIZE,
                              100 / portTICK_RATE_MS);
    if (len > 0)
    {
        ESP_LOGW(__FUNCTION__, "=============================================");
        ESP_LOGW(__FUNCTION__, "Received %u bytes:", len);
        ESP_LOG_BUFFER_HEX_LEVEL(__FUNCTION__, data, len, ESP_LOG_INFO);
        ESP_LOGW(__FUNCTION__, "=============================================");
        return true;
    }
    return false;
}

/**
 * @brief Caculator check sum crc16
 * @param uint8_t *buff: point to start address caculator
 * @return CRC value
 */
int crc16(uint8_t *buff, int len)
{
    unsigned int i, j, cs;
    cs = INIT_CHECKSUM;
    for (j = 0; j < len; j++)
    {
        cs = cs ^ buff[j];
        for (i = 0; i < 8; i++)
        {
            if (((cs) & 0x0001) == 1)
                cs = (cs >> 1) ^ CRC16_MODBUS;
            else
                cs = cs >> 1;
        }
    }
    return cs;
}

/**
 * @brief Function initRS485, creat queue and lock mutex
 */
void modbusInit()
{
    initRs485();
    queuePacketRS485 =
        xQueueCreate(MAX_QUEUE_PACKET_RS485, sizeof(queuePacketOutRS485_t));
    if (queuePacketRS485 == NULL)
    {
        DEBUG_LOGE(__FUNCTION__, "Error create queue");
        esp_restart();
    }

    xMutexModbusPort = xSemaphoreCreateMutex();
    if (xMutexModbusPort == NULL)
    {
        ESP_LOGE(__FUNCTION__, "False creat Semaphore!");
        esp_restart();
    }

    xMutexStatusStm32 = xSemaphoreCreateMutex();
    if (xMutexStatusStm32 == NULL)
    {
        ESP_LOGE(__FUNCTION__, "False creat Semaphore!");
        esp_restart();
    }
}

/**
 * @brief Function check modbus line free before timeout 10sec
 * @param bool status: status modbus line if have data it true otherwise false
 */
void checkLineModbus(bool status)
{
    static uint32_t timeCheck = 0;
    static uint32_t detal = xTaskGetTickCount();
    if (!status)
    {
        // check time
        detal = xTaskGetTickCount() - detal;
        timeCheck = timeCheck + detal;
        detal = xTaskGetTickCount();
        // set flag
        if ((timeCheck >= TIMEOUT_MODBUS) && (flagModbusEmpty == false))
        {
            timeCheck = 0;
            flagModbusEmpty = true;
            printf("------------------Modbus Free!\n");
        }
    }
    else
    {
        flagModbusEmpty = false;
        timeCheck = 0;
        detal = xTaskGetTickCount();
    }
}