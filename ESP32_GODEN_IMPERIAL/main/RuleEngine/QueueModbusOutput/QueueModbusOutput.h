#ifndef QUEUEMODBUSOUTPUT_H
#define QUEUEMODBUSOUTPUT_H

#include "Global.h"


// extern QueueHandle_t queueModbusOutput;

#define MAX_QUEUE_OUPUT 100
#define TICK_TO_WAIT_QUEUE (100 / portTICK_RATE_MS)

#ifdef __cplusplus
extern "C"
{
#endif

    void queueModbusOutputInit();

    uint8_t queueModbusOutputPush(outputItem_t *input);

    uint8_t queueModbusOutputPop(outputItem_t *output);

    uint8_t checkQueueOutputEmpty();
#ifdef __cplusplus
}
#endif
#endif