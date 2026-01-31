#ifndef __QUEUEINPUT_H_
#define __QUEUEINPUT_H_

#include "Global.h"


#define MAX_QUEUE_INPUT 100
#define TICK_TO_WAIT_QUEUE 10
#ifdef __cplusplus
extern "C"
{
#endif

    void queueInputInit();

    uint8_t queueInputPush(inputItem_t *input);

    uint8_t queueInputPop(inputItem_t *output);

#ifdef __cplusplus
}
#endif

#endif
