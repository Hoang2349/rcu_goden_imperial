#include "HandleOutputQueue.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "MapRule.h"
#include "MapRuleMqtt.h"
#include "Modbus.h"
#include "QueueModbusOutput.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

outputItem_t output;
uint8_t dataOutput[SIZE_DATA_CONTROL] = {0};

inforDataModbusSend_t buffferBatch[MAX_PACKGE_PARSE] = {0};

/**
 * @brief Function hanlde batching data to bufferBatch
 * @param inforDataModbusSend_t input store data need handle
 * @return true or false (new address slave or haved)
 */
static bool pushDataToBatch(inforDataModbusSend_t input)
{
    for (int i = 0; i < MAX_PACKGE_PARSE; i++)
    {
        if (input.address == buffferBatch[i].address &&
            input.reg == buffferBatch[i].reg)
        {
            buffferBatch[i].value = input.value;
            ESP_LOGW(__FUNCTION__, "Duplicate Packge!\n");
            return false;
        }
    }

    for (int i = 0; i < MAX_PACKGE_PARSE; i++)
    {
        if ((buffferBatch[i].address == 0) && (buffferBatch[i].reg == 0))
        {
            buffferBatch[i].address = input.address;
            buffferBatch[i].reg = input.reg;
            buffferBatch[i].value = input.value;
            ESP_LOGI(__FUNCTION__, "Add Packge success\n");
            break;
        }
    }
    return true;
}

/**
 * @brief Function hanlde push data to modbus from bufferBatch
 */
static void pushDataBatchToModbus()
{
    for (int i = 0; i < MAX_PACKGE_PARSE; i++)
    {
        if (buffferBatch[i].address != 0 && buffferBatch[i].reg != 0)
        {
            dataOutput[SLAVE_ADDR] = buffferBatch[i].address;
            dataOutput[FUNC] = MB_FC_WRITE_REGISTER;
            dataOutput[REG_H] = buffferBatch[i].reg >> 8;
            dataOutput[REG_L] = buffferBatch[i].reg & 0xFF;
            dataOutput[DATA_H] = buffferBatch[i].value >> 8;
            dataOutput[DATA_L] = buffferBatch[i].value & 0xFF;
            fillCRC((uint8_t *)dataOutput, SIZE_DATA_CONTROL);
            sendDataModbus((uint8_t *)dataOutput, SIZE_DATA_CONTROL,
                           TIMEOUT_RESPONSE);
            // printf("Send data: addess->0x%X   Reg:->0x%X\n",
            //    buffferBatch[i].address, buffferBatch[i].reg);
            buffferBatch[i].address = 0;
            buffferBatch[i].reg = 0;
            buffferBatch[i].value = 0;
        }
    }
}
/**
 * @brief Function Pop data in Queue Output parse  after push to Modbus slave
 * @param None
 * @return None
 */
void HandleOutputQueue()
{
    while (1)
    {
        if (checkQueueOutputEmpty())
        {
            vTaskDelay(10 / portTICK_RATE_MS);
            continue;
        }
        uint8_t countPackge = 0;
        TickType_t tickCount = xTaskGetTickCount();
        TickType_t deltaCount = 0;
        do
        {
            inforDataModbusSend_t batch = {0};
            // pop data in queue
            if (!queueModbusOutputPop(&output))
            {
                vTaskDelay(10 / portTICK_RATE_MS);
                break;
            }

            // handle type device on/off
            if (output.type == outputRj45 || output.type == relay ||
                output.type == curtainRelay || output.type == curtainRj45 ||
                output.type == ledModbus)
            {
                // compare addr and register
                char str[9] = {0};
                char key[7] = {0};
                snprintf(str, sizeof(str), "%08lX", output.address);
                // coppy 6 char first from 'str' to key
                for (int i = 0; i < 6; i++)
                {
                    key[i] = str[i];
                }
                // set bit end = empty
                key[6] = '\0';
                // get value reg on map
                uint16_t oldValue = 0;
                getRegister(&key[0], &oldValue);
                // set new value for reg
                uint8_t locationBitSet = output.address & 0xff;
                batch.address = (output.address >> (8 * 3)) & 0xff;
                batch.reg = (output.address >> 8) & 0xffff;
                fillDataSlaveAddress(output.value, oldValue, locationBitSet,
                                     &batch.value);
                insertRegister(&key[0], batch.value);
                // push to queue Bath
                pushDataToBatch(batch);
            }
            // handle for dimmer
            else if (output.type == dimmer)
            {
                dataOutput[SLAVE_ADDR] = (output.address >> 24) & 0xff;
                dataOutput[FUNC] = MB_FC_WRITE_REGISTER;
                dataOutput[REG_H] = (output.address >> 16) & 0xff;
                dataOutput[REG_L] = (output.address >> 8) & 0xff;
                dataOutput[DATA_H] = 0x00;
                dataOutput[DATA_L] = output.value;
                fillCRC((uint8_t *)dataOutput, SIZE_DATA_CONTROL);
                sendDataModbus((uint8_t *)dataOutput, SIZE_DATA_CONTROL,
                               TIMEOUT_RESPONSE);
            }

            // handle for aircoordinator
            else if (output.type == airCoodinatorIR)
            {
                dataOutput[SLAVE_ADDR] = (output.address >> (8 * 3)) & 0xff;
                dataOutput[FUNC] = MB_FC_WRITE_REGISTER;
                dataOutput[REG_H] = (output.address >> (8 * 2)) & 0xff;
                dataOutput[REG_L] = (output.address >> (8 * 1)) & 0xff;
                dataOutput[DATA_H] = 0x00;
                dataOutput[DATA_L] = output.value;
                fillCRC((uint8_t *)dataOutput, SIZE_DATA_CONTROL);
                sendDataModbus((uint8_t *)dataOutput, SIZE_DATA_CONTROL,
                               TIMEOUT_RESPONSE);
            }
            countPackge++;
            deltaCount = xTaskGetTickCount() - tickCount;
        } while ((deltaCount < TIME_SCAN) && (countPackge < MAX_PACKGE_PARSE));
        // printf("Tick Count: %lu\n", deltaCount);
        // printf("Packge Count: %d\n", countPackge);
        // push data to queue MB
        pushDataBatchToModbus();
    }
}

/**
 * @brief ham set gia tri cua thanh ghi
 * @param uint8_t state trang thai dieu khioen mong muon cua
 * @param uint16_t regIn gia tri thanh ghi hien tai
 * @return NULL
 */
void fillDataSlaveAddress(uint8_t state, uint16_t regIn, uint8_t location,
                          uint16_t *regOut)
{
    uint16_t regOutTmp = 0;
    location = location - 1;
    if (state == RULE_OFF)
    {
        regOutTmp = regIn & (~(1 << location));
    }
    if (state == RULE_ON)
    {
        regOutTmp = regIn | (1 << location);
    }
    if (state == RULE_TOGGLE)
    {
        regOutTmp = regIn ^ (1 << location);
    }
    *regOut = regOutTmp;
}

/**
 * @brief ham xu li lay gia tri cuar byte trong bieesn uint32
 * @param uint32_t input bien nguon
 * @param uint8_t location vi byte can lay gia trij
 * @return gia tri byte can lay
 */
uint8_t getByteFromVariable(uint32_t input, uint8_t location)
{
    return ((input >> (8 * location)) & 0xff);
}

/**
 * @brief ham tinh CRC16 bit cho modbus
 * @param uint8_t *buff dia chi bat dau data can tinh
 * @param int len do dai cua data
 * @return gia tri byte can lay
 */
uint16_t calCRC16(uint8_t *buff, int len)
{
    uint16_t i, j, cs;
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
 * @brief them cac bytr CRC vao farme
 * @param uint8_t *buff dia chi bat dau data can tinh
 * @param int len do dai cua data
 * @return gia tri byte can lay
 */
void fillCRC(uint8_t *buff, int len)
{
    uint16_t crc = calCRC16((uint8_t *)buff, len - sizeof(uint16_t));
    *(buff + len - 2) = LO_UINT16(crc);
    *(buff + len - 1) = HI_UINT16(crc);
}

/// @brief  handle output data
/// @param arg no use
void handleOutput(void *arg)
{
    while (1)
    {
        HandleOutputQueue();
        vTaskDelay(HANDLE_OUPUT_DELAY_TASK / portTICK_RATE_MS);
    }
}
