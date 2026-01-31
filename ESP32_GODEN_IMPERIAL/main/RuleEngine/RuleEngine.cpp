#include "RuleEngine.h"

#include "AppGpio.h"
#include "AppSpiffs.h"
#include "Global.h"
#include "HandleOutputQueue.h"
#include "MapRule.h"
#include "MapRuleMqtt.h"
#include "Modbus.h"
#include "Mqtt.h"
#include "MqttDeviceData.h"
#include "QueueInput.h"
#include "QueueModbusOutput.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "string.h"
TaskHandle_t xRuleLocal;
TaskHandle_t xRuleMqtt;
TaskHandle_t xledRuleLocal;
TaskHandle_t xledRuleMqtt;
SemaphoreHandle_t xMutexHandleRule;
attributesRule_t ruleHandle = {true, RULE_MQTT, RULE_MQTT, 0};

/// @brief main task rule local process
/// @param arg no handle
void ruleEngineLocal(void *arg)
{
    while (1)
    {
        inputItem_t val;
        rule_t scen = {};

        if (queueInputPop(&val) == RULE_ERROR)
        {
            continue;
        }

        // Max length key get from address type int
        char key[9];
        // convert uint32_t to key string
        snprintf(key, sizeof(key), "%08lX", val.address);
        ESP_LOGW(__FUNCTION__, "key: %s", key);

        // get value from key in Map Scenario
        getScenario(key, &scen);
        // displayRe(scen);
        for (auto it = scen.output.begin(); it != scen.output.end(); ++it)
        {
            outputItem_t data;
            data.type = it->type;
            data.address = it->address;
            data.value = it->value;
            queueModbusOutputPush(&data);
        }
        vTaskDelay(RULE_ENGINE_DELAY_TASK / portTICK_PERIOD_MS);
    }
}

/// @brief Main task rule MQTT online
/// @param arg no use
void ruleEngineMqtt(void *arg)
{
    while (1)
    {
        inputItem_t val;
        rule_t scen = {};

        if (queueInputPop(&val) == RULE_ERROR)
        {
            continue;
        }
        // parse pin device lấy thông tin địa chỉ và pin
        uint8_t address = 0;
        uint8_t pin = 0;
        uint8_t value = 0;

        address = (uint8_t)(val.address >> 24);
        pin = (uint8_t)(val.address & 0x0000FF);
        value = val.value;
        uint16_t key = (uint16_t)address << 8 | pin;

        // handle switch rs485
        if (address != RCU_MODBUS_ADDRESS_DEFAULT)
        {
            handldeSwitchRs485DeviceData(address, pin, value);
            continue;
        }

        // handle switch rj45

        int typeInput = getRcuInputToType((int)key);
        switch (typeInput)
        {
            case switchRj45:
            {
                handldeSwitchDeviceData(address, pin, value);
            }
            break;
            case motionRj45:
            {
                handldeMotionDeviceData(address, pin, value);
            }
            break;
            case doorRj45:
            {
                handleDoorDeviceData(address, pin, value);
            }
            break;
            default:
                break;
        }
        vTaskDelay(RULE_ENGINE_DELAY_TASK / portTICK_PERIOD_MS);
    }
}

/// @brief task handle run main taask active
/// @param arg no use
void handleRuleEngine(void *arg)
{
    xMutexHandleRule = xSemaphoreCreateMutex();
    if (xMutexHandleRule != NULL)
    {
        // to do handle
    }

    while (1)
    {
        if (xSemaphoreTake(xMutexHandleRule, (TickType_t)10) == pdTRUE)
        {
            // handle Rule
            if (ruleHandle.flagRuleEngine == false)
            {
                goto waitTime;
            }
            if (ruleHandle.ruleCurrent == ruleHandle.ruleNew)
            {
                ruleHandle.flagRuleEngine = false;
                ruleHandle.tickWait = 0;
                goto waitTime;
            }
            if (ruleHandle.ruleNew == RULE_MQTT)
            {
                ruleHandle.tickWait = TIMEOUT_CHANGE_MODE;
            }
            // cộng dồn thời gian timeOut = tickWait(sec)
            ESP_LOGW(__FUNCTION__, "time change Mode RuleEngine: %d",
                     ruleHandle.tickWait);
            if (ruleHandle.tickWait < TIMEOUT_CHANGE_MODE)
            {
                ruleHandle.tickWait++;
                goto waitTime;
            }
            ruleHandle.tickWait = 0;
            if (ruleHandle.ruleCurrent == RULE_LOCAL)
            {
                ESP_LOGW(__FUNCTION__,
                         "---------------Running Task Rule MQTT--------------");
                if (xRuleLocal != NULL)
                {
                    vTaskSuspend(xRuleLocal);
                    vTaskSuspend(xledRuleLocal);
                }
                vTaskResume(xRuleMqtt);
                vTaskResume(xledRuleMqtt);
            }
            else if (ruleHandle.ruleCurrent == RULE_MQTT)
            {
                ESP_LOGW(
                    __FUNCTION__,
                    "---------------Running Task Rule Local--------------");
                if (xRuleMqtt != NULL)
                {
                    vTaskSuspend(xRuleMqtt);
                    vTaskSuspend(xledRuleMqtt);
                }
                vTaskResume(xRuleLocal);
                vTaskResume(xledRuleLocal);
            }
            ruleHandle.flagRuleEngine = false;
            ruleHandle.ruleCurrent = ruleHandle.ruleNew;
        }
    waitTime:
        xSemaphoreGive(xMutexHandleRule);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/// @brief led display rule local
/// @param arg no use
void ledRuleLocal(void *arg)
{
    while (1)
    {
        ledStatusRuleLocal();
    }
}

/// @brief led display rule mqtt
/// @param arg no use
void ledRuleMqtt(void *arg)
{
    while (1)
    {
        ledStatusRuleMqtt();
    }
}

/// @brief Funciton init creat task, queue, and run file init for rule onl,
/// offline
/// @param
void ruleEngineInit(void)
{
    queueInputInit();
    queueModbusOutputInit();
    xTaskCreate(handleOutput, "Task Handle Output", 4096, NULL, 4, NULL);
    xTaskCreate(handleRs485, "Rs485 Handle TX-Rx", 4096, NULL, 6, NULL);
    xTaskCreate(ledRuleLocal, "Led Rule Local", 1024 * 2, NULL, 2,
                &xledRuleLocal);
    vTaskSuspend(xledRuleLocal);
    xTaskCreate(ledRuleMqtt, "Led Rule Mqtt", 1024 * 2, NULL, 2, &xledRuleMqtt);
}

/// @brief ffunction push data to queue Output
/// @param type : type deive
/// @param address : address  need set
/// @param value : value need set
void pushDataToQueueOutput(uint8_t type, uint32_t address, uint8_t value)
{
    outputItem_t data;
    data.type = type;
    data.address = address;
    data.value = value;
    ESP_LOGI(__FUNCTION__, "Data Value: %d", data.value);
    queueModbusOutputPush(&data);
}

/// @brief log heap free to mqtt
void logDebugHeap()
{
    static unsigned int count123 = 0;
    ESP_LOGW(__FUNCTION__, "Number Check : %d", count123++);
    uint32_t total_heap_size = esp_get_free_heap_size();
    ESP_LOGI(__FUNCTION__, "Total heap free size: %ld bytes", total_heap_size);
    // pushInforHeap(total_heap_size);
}
