#include "QueueModbusOutput.h"

QueueHandle_t queueModbusOutput;

/**
 * @brief Ham khoi tao queue ouput mobus
 * @param None
 * @return None
 */
void queueModbusOutputInit()
{
    queueModbusOutput = xQueueCreate(MAX_QUEUE_OUPUT, sizeof(outputItem_t));
    if (queueModbusOutput == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Error create queue");
        esp_restart();
    }
}

/**
 * @brief Ham truyen ouput vao queue
 * @param inputItem_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueModbusOutputPush(outputItem_t *input)
{
    if (xQueueSendToBack(queueModbusOutput, (void *)input,
                         TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        ESP_LOGE(__FUNCTION__, "Error xQueueSendToBack = errQUEUE_FULL");
        return RULE_ERROR;
    }
    return RULE_OK;
}

/**
 * @brief Ham truyen lay output tu queue
 * @param outputItem_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueModbusOutputPop(outputItem_t *output)
{
    if (xQueueReceive(queueModbusOutput, output, TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        ESP_LOGE(__FUNCTION__, "Error xQueueSendToBack = pdFALSE");
        return RULE_ERROR;
    }
    return RULE_OK;
}

/// @brief Check have data on queue
/// @return  ok or false
uint8_t checkQueueOutputEmpty()
{
    if (uxQueueMessagesWaiting(queueModbusOutput) == 0)
    {
        return RULE_OK;
    }
    return RULE_ERROR;
}