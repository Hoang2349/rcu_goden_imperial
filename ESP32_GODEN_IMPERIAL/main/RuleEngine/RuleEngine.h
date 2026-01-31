#ifndef __RULEENGINE_H_
#define __RULEENGINE_H_

#include <stdint.h>
#include <stdio.h>

#define RULE_ENGINE_DELAY_TASK (5)

#define TIMEOUT_CHANGE_MODE (30)

typedef struct
{
    bool flagRuleEngine;
    int ruleCurrent;
    int ruleNew;
    int tickWait;
} attributesRule_t;

enum ruleMode
{
    RULE_LOCAL = 1,
    RULE_MQTT = 2,
};

#ifdef __cplusplus
extern "C"
{
#endif
    void ruleEngineInit(void);
    void pushDataToQueueOutput(uint8_t type, uint32_t address, uint8_t value);
    void logDebugHeap();
#ifdef __cplusplus
}
#endif
#endif