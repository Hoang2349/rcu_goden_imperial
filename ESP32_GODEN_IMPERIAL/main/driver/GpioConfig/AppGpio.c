#include "AppGpio.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TIME_LED_ON_RULE_ONLINE (200)
#define TIME_PERIOD_RULE_ONLINE (1000)
#define TIME_PERIOD_RULE_OFFLINE (1000)
#define COUNT_ON_OFF_RULE_OFFLINE (3)

/**
 * @param Function process blink led term sequence 2s on\1s off\1s
 */
void ledStatusRuleMqtt()
{
    gpio_set_level(GPIO_LED_STATUS_RULE, OFF);
    vTaskDelay(TIME_PERIOD_RULE_OFFLINE / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_LED_STATUS_RULE, ON);
    vTaskDelay(TIME_PERIOD_RULE_OFFLINE / portTICK_PERIOD_MS);
}

/**
 * @param Function process blink led on/off 3 times on 300ms next off 1s
 */
void ledStatusRuleLocal()
{
    for (int i = 0; i < COUNT_ON_OFF_RULE_OFFLINE; i++)
    {
        gpio_set_level(GPIO_LED_STATUS_RULE, OFF);
        vTaskDelay(TIME_LED_ON_RULE_ONLINE / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_LED_STATUS_RULE, ON);
        vTaskDelay(TIME_LED_ON_RULE_ONLINE / portTICK_PERIOD_MS);
    }
    vTaskDelay(TIME_PERIOD_RULE_ONLINE / portTICK_PERIOD_MS);
}

/**
 * @param Function init gpio mode Ouput, pull down, disable interrupt
 */
void appSetupGpio()
{
    gpio_config_t gpioConfig = {.intr_type = GPIO_INTR_DISABLE,
                                .mode = GPIO_MODE_OUTPUT,
                                .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
                                .pull_up_en = 0,
                                .pull_down_en = 1};
    gpio_config(&gpioConfig);
}
