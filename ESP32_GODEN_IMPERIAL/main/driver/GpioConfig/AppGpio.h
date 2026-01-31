#ifndef __APP_GPIO_H_
#define __APP_GPIO_H_

#include <stdbool.h>
#include <stdio.h>

// GPIO PIN Discriptions
#define GPIO_LED_STATUS_RULE (GPIO_NUM_2)
#define GPIO_OUTPUT_PIN_SEL (1ULL << GPIO_LED_STATUS_RULE)
// #define GPIO_RESET1_STM32 GPIO_NUM_15
// #define GPIO_RESET2_STM32 GPIO_NUM_4

enum stateGpio
{
    ON = 1,
    OFF = 0
};

#ifdef __cplusplus
extern "C"
{
#endif
    void appSetupGpio();
    void ledStatusRuleMqtt();
    void ledStatusRuleLocal();
#ifdef __cplusplus
}
#endif

#endif