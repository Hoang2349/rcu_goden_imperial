#include "QueueInput.h"


QueueHandle_t queueInput;

// #define DEBUG_LOGE ESP_LOGE

/**
 * @brief Ham khoi tao queue input
 * @param None
 * @return None
 */
void queueInputInit()
{
    queueInput = xQueueCreate(MAX_QUEUE_INPUT, sizeof(inputItem_t));
    if (queueInput == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Error create queue");
        esp_restart();
    }
}

/**
 * @brief Ham truyen ouput vao queue
 * @param variableOutput_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueInputPush(inputItem_t *input)
{
    if (xQueueSendToBack(queueInput, (void *)input,
                         (TickType_t)TICK_TO_WAIT_QUEUE) != pdPASS)
    {
        // ESP_LOGE(__FUNCTION__, "Error xQueueSendToBack = errQUEUE_FULL");
        return RULE_ERROR;
    }
    else
    {
        ESP_LOGI(__FUNCTION__, "Push data Success");
        return RULE_OK;
    }
}

/**
 * @brief Ham truyen lay output tu queue
 * @param variableOutput_t* - Con tro toi truct output
 * @return None
 */
uint8_t queueInputPop(inputItem_t *output)
{
    if (xQueueReceive(queueInput, output, (TickType_t)TICK_TO_WAIT_QUEUE) !=
        pdPASS)
    {
        // ESP_LOGE(__FUNCTION__, "Error xQueueRecive Queue Empty!");
        return RULE_ERROR;
    }
    else
    {
        ESP_LOGI(__FUNCTION__, "Pop data Success");
        return RULE_OK;
    }
}
