#include "MqttDeviceCommand.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "MapRuleMqtt.h"
#include "Mqtt.h"
#include "MqttDeviceData.h"
#include "RuleEngine.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h "
#include "freertos/task.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tahu.h"
#include "tahu_pb.h"

attributeDevice_t f_device_open;
attributeDevice_t f_device_close;

QueueHandle_t xQueueOpenCurtain = NULL;
QueueHandle_t xQueueCloseCurtain = NULL;
/// @brief Function handle data recive data form server
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parseDataMqttSparkplug(sMqttPkg_t dataInput)
{
    ESP_LOGW(__FUNCTION__, "---------Handle data recive-------");
    // parse Topic xác định type device id
    char *deviceId = strrchr(dataInput.strTopic, '/');
    if (deviceId != NULL)
    {
        // Nếu tìm thấy, in ra phần sau ký tự '/' cuối cùng
        deviceId = deviceId + 1;
        // get type handle from deviceiD
        int key = atoi(deviceId);
        dataInput.deviceId = key;
        attributeDevice_t device = getDeviceIdToAttribute(key);
        switch (device.type)
        {
            case outputRj45:
            {
                static int count = 0;
                ESP_LOGI(__FUNCTION__, "Control Output RJ45");
                parsePayloadRJ45OutputDcmd(device, dataInput);
                break;
            }
            case relay:
            {
                ESP_LOGI(__FUNCTION__, "Control Output Relay");
                parsePayloadRelayOnBoardDcmd(device, dataInput);
                break;
            }
            case ledModbus:
            {
                parsePayloadRS485LedDcmd(device, dataInput);
                ESP_LOGI(__FUNCTION__, "Control Output LedModbus");
                break;
            }
            case curtainRelay:
            {
                // parse controll Rem
                parsePayloadCurtainDcmd(device, dataInput);
                ESP_LOGI(__FUNCTION__, "Control Curtain ");
                break;
            }
            case curtainRj45:
            {
                // parse control Rem
                parsePayloadCurtainDcmd(device, dataInput);
                ESP_LOGI(__FUNCTION__, "Control Curtain ");
                break;
            }
            case airCoodinatorIR:
            {
                parsePayloadAirCoodinatorIR(device, dataInput);
                ESP_LOGI(__FUNCTION__, "Control AirCoodinator");
                break;
            }
            case dimmer:
            {
                parsePayloadDimmer(device, dataInput);
                break;
            }
            default:
                ESP_LOGW(__FUNCTION__, "No Handle Device Type !!!");
                break;
        }
        // todo handle deviceId
    }
    return;
}

/// @brief parse data and control relay onboard
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadRelayOnBoardDcmd(attributeDevice_t device,
                                  sMqttPkg_t dataInput)
{
    uint8_t addr = RCU_MODBUS_ADDRESS_DEFAULT;  // mặc định RCU
    uint8_t reg = RELAY_REGISTER_DEFAULT;
    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;
    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        return;
    }
    uint32_t address =
        (uint32_t)addr << 24 | (uint32_t)reg << 8 | (uint32_t)device.pin;
    uint8_t value = dataPayload.metrics[0].value.boolean_value;

    // check name mettrics 1:attributes/*  2:states/*
    if (dataPayload.metrics_count != 1)
    {
        free_payload(&dataPayload);
        return;
    }
    if (strcmp(dataPayload.metrics[0].name, "states/on") == 0)
    {
        pushDataToQueueOutput(relay, address, value);
        free_payload(&dataPayload);
        return;
    }
    else if (strcmp(dataPayload.metrics[0].name, "commands/OnOff/on") == 0)
    {
        pushDataToQueueOutput(relay, address, value);
        handldeRelayDeviceData(dataInput.deviceId, value);
        free_payload(&dataPayload);
        return;
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "Not Support name Metrict");
    }
    free_payload(&dataPayload);
    // to do
}

/// @brief Parse data control RJ45 output
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadRJ45OutputDcmd(attributeDevice_t device, sMqttPkg_t dataInput)
{
    uint8_t addr = RCU_MODBUS_ADDRESS_DEFAULT;  // mặc định RCU
    uint8_t reg1 = RJ45_REGISTER1_DEFAULT;
    uint8_t reg2 = RJ45_REGISTER2_DEFAULT;

    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;
    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        return;
    }

    uint32_t address = 0;
    uint8_t pin = device.pin;
    uint8_t value = dataPayload.metrics[0].value.boolean_value;

    if (pin > 0 && pin < MAX_NUMBER_PIN_PORT1)
    {
        address = (uint32_t)addr << 24 | (uint32_t)reg1 << 8 | (uint32_t)pin;
    }
    if (pin >= MAX_NUMBER_PIN_PORT1 && pin <= MAX_NUMBER_PIN_PORT2)
    {
        pin = pin - DELTA_GET_BIT_CONTROL;
        address = (uint32_t)addr << 24 | (uint32_t)reg2 << 8 | (uint32_t)pin;
    }
    // Suport Message 1 metrics
    if (dataPayload.metrics_count != MAX_METRICS_HANDLE)
    {
        free_payload(&dataPayload);
        return;
    }
    if (strcmp(dataPayload.metrics[0].name, "states/on") == 0)
    {
        pushDataToQueueOutput(outputRj45, address, value);
        free_payload(&dataPayload);
        return;
    }
    else if (strcmp(dataPayload.metrics[0].name, "commands/OnOff/on") == 0)
    {
        pushDataToQueueOutput(outputRj45, address, value);
        handldeRelayDeviceData(dataInput.deviceId, value);
        free_payload(&dataPayload);
        return;
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "Not Support name Metrict");
    }
    free_payload(&dataPayload);
    // to do
}

/// @brief Parse data output led RS485
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadRS485LedDcmd(attributeDevice_t device, sMqttPkg_t dataInput)
{
    int addr = device.address;
    uint16_t reg = LED_MODBUS_REGISTER_DEFAULT;  // dia chi thanh ghi relay
    int pin = device.pin;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;
    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        return;
    }
    // push data to queue out
    uint32_t address =
        (uint32_t)addr << 24 | (uint32_t)reg << 8 | (uint32_t)pin;
    uint8_t value = dataPayload.metrics[0].value.boolean_value;
    // check name mettrics 1:attributes/*  2:states/*
    if (dataPayload.metrics_count != 1)
    {
        free_payload(&dataPayload);
        return;
    }
    if (strcmp(dataPayload.metrics[0].name, "states/on") == 0)
    {
        pushDataToQueueOutput(ledModbus, address, value);
        free_payload(&dataPayload);
        return;
    }
    else if (strcmp(dataPayload.metrics[0].name, "commands/OnOff/on") == 0)
    {
        pushDataToQueueOutput(ledModbus, address, value);
        handldeRelayDeviceData(dataInput.deviceId, value);
        free_payload(&dataPayload);
        return;
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "Not Support name Metrict");
    }
    free_payload(&dataPayload);
}

/// @brief Task action open curtain
/// @param pvParameters: no use
void taskCurtainOpen(void *pvParameters)
{
    attributeDevice_t device = {0};
    uint8_t regOpen = 0;
    uint8_t regClose = 0;
    while (1)
    {
        if (xQueueReceive(xQueueOpenCurtain, &(device),
                          TIME_WAIT_TASK_CURTAIN) == pdPASS)
        {
            if (device.type == curtainRelay)
            {
                regOpen = RELAY_REGISTER_DEFAULT;
                regClose = RELAY_REGISTER_DEFAULT;
            }
            else if (device.type == curtainRj45)
            {
                // lay dia chij thanh ghi pinOpen
                if (device.pinOpen > 0 && device.pinOpen < MAX_NUMBER_PIN_PORT1)
                {
                    regOpen = RJ45_REGISTER1_DEFAULT;
                }
                else if (device.pinOpen >= MAX_NUMBER_PIN_PORT1 &&
                         device.pinOpen <= MAX_NUMBER_PIN_PORT2)
                {
                    regOpen = RJ45_REGISTER2_DEFAULT;
                    device.pinOpen = device.pinOpen - DELTA_GET_BIT_CONTROL;
                }
                // lay dia chij thanh ghi pinclose
                if (device.pinClose > 0 &&
                    device.pinClose < MAX_NUMBER_PIN_PORT1)
                {
                    regClose = RJ45_REGISTER1_DEFAULT;
                }
                else if (device.pinClose >= MAX_NUMBER_PIN_PORT1 &&
                         device.pinClose <= MAX_NUMBER_PIN_PORT2)
                {
                    regClose = RJ45_REGISTER2_DEFAULT;
                    device.pinClose = device.pinClose - DELTA_GET_BIT_CONTROL;
                }
            }
            uint32_t addressOpen = (uint32_t)RCU_MODBUS_ADDRESS_DEFAULT << 24 |
                                   (uint32_t)regOpen << 8 |
                                   (uint32_t)device.pinOpen;

            uint32_t addressClose = (uint32_t)RCU_MODBUS_ADDRESS_DEFAULT << 24 |
                                    (uint32_t)regClose << 8 |
                                    (uint32_t)device.pinClose;
            pushDataToQueueOutput(device.type, addressOpen, ACTIVE_RELAY);
            vTaskDelay(pdMS_TO_TICKS(device.timeOut));
            pushDataToQueueOutput(device.type, addressClose, UNACTIVE_RELAY);
            pushDataToQueueOutput(device.type, addressOpen, UNACTIVE_RELAY);
        }
        vTaskDelay(TIME_WAIT_TASK_CURTAIN);
    }
}

/// @brief Task action close curtain
/// @param pvParameters: no use
void taskCurtainClose(void *pvParameters)
{
    attributeDevice_t device = {0};
    uint32_t regOpen = 0;
    uint32_t regClose = 0;
    while (1)
    {
        if (xQueueReceive(xQueueCloseCurtain, &(device),
                          TIME_WAIT_TASK_CURTAIN) == pdPASS)
        {
            if (device.type == curtainRelay)
            {
                regOpen = RELAY_REGISTER_DEFAULT;
                regClose = RELAY_REGISTER_DEFAULT;
            }
            else if (device.type == curtainRj45)
            {
                // lay dia chij thanh ghi pinOpen
                if ((device.pinOpen > 0) &&
                    (device.pinOpen < MAX_NUMBER_PIN_PORT1))
                {
                    regOpen = RJ45_REGISTER1_DEFAULT;
                }
                else if ((device.pinOpen >= MAX_NUMBER_PIN_PORT1) &&
                         (device.pinOpen <= MAX_NUMBER_PIN_PORT2))
                {
                    regOpen = RJ45_REGISTER2_DEFAULT;
                    device.pinOpen = device.pinOpen - DELTA_GET_BIT_CONTROL;
                }
                // lay dia chij thanh ghi pinclose
                if (device.pinClose > 0 &&
                    (device.pinClose < MAX_NUMBER_PIN_PORT1))
                {
                    regClose = RJ45_REGISTER1_DEFAULT;
                }
                else if (device.pinClose >= MAX_NUMBER_PIN_PORT1 &&
                         (device.pinClose <= MAX_NUMBER_PIN_PORT2))
                {
                    regClose = RJ45_REGISTER2_DEFAULT;
                    device.pinClose = device.pinClose - DELTA_GET_BIT_CONTROL;
                }
            }
            uint32_t addressOpen = (uint32_t)RCU_MODBUS_ADDRESS_DEFAULT << 24 |
                                   (uint32_t)regOpen << 8 |
                                   (uint32_t)device.pinOpen;

            uint32_t addressClose = (uint32_t)RCU_MODBUS_ADDRESS_DEFAULT << 24 |
                                    (uint32_t)regClose << 8 |
                                    (uint32_t)device.pinClose;
            pushDataToQueueOutput(device.type, addressClose, ACTIVE_RELAY);
            vTaskDelay(pdMS_TO_TICKS(device.timeOut));
            pushDataToQueueOutput(device.type, addressClose, UNACTIVE_RELAY);
            pushDataToQueueOutput(device.type, addressOpen, UNACTIVE_RELAY);
        }
        vTaskDelay(TIME_WAIT_TASK_CURTAIN);
    }

    // printf("Curtain close stop\n");
}

/// @brief Init Task Open/close Curtain
void initRcuCurtain()
{
    // creat handle queue save action curtain
    xQueueOpenCurtain = xQueueCreate(5, sizeof(attributeDevice_t));
    xQueueCloseCurtain = xQueueCreate(5, sizeof(attributeDevice_t));
    if ((xQueueOpenCurtain == NULL) || (xQueueCloseCurtain == NULL))
    {
        ESP_LOGE(__FUNCTION__, "Error create queue");
        esp_restart();
    }
    // creat static two task handle open and close curtain
    xTaskCreate(taskCurtainClose, "Curtain Close Task", 2048, NULL, 2, NULL);
    xTaskCreate(taskCurtainOpen, "Curtain Open Task", 2048, NULL, 2, NULL);
}

// handle Curtain Open and Close

/// @brief Parse data control curtain from mqtt
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadCurtainDcmd(attributeDevice_t device, sMqttPkg_t dataInput)
{
    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;
    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        return;
    }
    if (dataPayload.metrics_count != 1)
    {
        free_payload(&dataPayload);
        return;
    }
    if ((strcmp(dataPayload.metrics[0].name,
                "states/openState/openDirection") != 0) &&
        (strcmp(dataPayload.metrics[0].name,
                "commands/OpenClose/openDirection") != 0))
    {
        ESP_LOGE(__FUNCTION__, "Not handle metrics!");
        free_payload(&dataPayload);
        return;
    }
    char *valueString = dataPayload.metrics[0].value.string_value;
    if (strcmp(valueString, "UP") == 0)
    {
        if (xQueueSendToBack(xQueueOpenCurtain, &device, (TickType_t)10) !=
            pdPASS)
        {
            ESP_LOGE(__FUNCTION__, "push queue open curtain");
        }
        handleCurtainDeviceData(dataInput.deviceId, valueString);
        free_payload(&dataPayload);
        return;
    }
    else if (strcmp(valueString, "DOWN") == 0)
    {
        if (xQueueSendToBack(xQueueCloseCurtain, &device, (TickType_t)10) !=
            pdPASS)
        {
            ESP_LOGE(__FUNCTION__, "push queue close curtain");
        }
        handleCurtainDeviceData(dataInput.deviceId, valueString);
        free_payload(&dataPayload);
        return;
    }
    free_payload(&dataPayload);
    return;
}

//--------------------Handle AirCoodinator---------------//

/// @brief  Parse handle control Aircoordinator
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadAirCoodinatorIR(attributeDevice_t device, sMqttPkg_t dataInput)
{
    // Parse Handle mqtt package
    ESP_LOGW(__FUNCTION__, "Handle AirCoodinator!");
    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;

    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        return;
    }
    if (dataPayload.metrics_count != 1)
    {
        free_payload(&dataPayload);
        return;
    }
    // check command type status/FanMode/activeMode/temporary
    if (strcmp(dataPayload.metrics[0].name, "states/on") == 0 ||
        strcmp(dataPayload.metrics[0].name, "commands/OnOff/on") == 0)
    {
        ESP_LOGI(__FUNCTION__, "ON OFF dieu hoa");
        int states = dataPayload.metrics[0].value.boolean_value;
        handleSetOnOffAirCoodinatorCommand(device, states);
        // push data to thangBoard
        handleAirCoodinatorStatusDeviceData(dataInput.deviceId, states);
    }

    // handle command active mode
    else if (strcmp(dataPayload.metrics[0].name,
                    "states/activeThermostatMode") == 0 ||
             strcmp(dataPayload.metrics[0].name,
                    "commands/ThermostatSetMode/thermostatMode") == 0)
    {
        ESP_LOGI(__FUNCTION__, "set mode cho dieu hoa");
        int mode = 0;
        char *valueString = dataPayload.metrics[0].value.string_value;
        if (strcmp(valueString, "auto") == 0)
        {
            mode = ACTIVE_AUTO;
        }
        else if (strcmp(valueString, "heat") == 0)
        {
            mode = ACTIVE_HEAT;
        }
        else if (strcmp(valueString, "cool") == 0)
        {
            mode = ACTIVE_COOL;
        }
        else if (strcmp(valueString, "fan-only") == 0)
        {
            mode = ACTIVE_FAN_ONLY;
        }
        else if (strcmp(valueString, "dry") == 0)
        {
            mode = ACTIVE_DRY;
        }
        // printf("Mode coordinator\n");
        handleActiveModeAirCoodinatorCommand(device, mode);
        // push data to thangBoard
        handleAirCoodinatorActiveModeDeviceData(dataInput.deviceId,
                                                valueString);
    }

    // handle comand mode fan
    else if (strcmp(dataPayload.metrics[0].name,
                    "states/currentFanSpeedSetting") == 0 ||
             strcmp(dataPayload.metrics[0].name,
                    "commands/SetFanSpeed/fanSpeed") == 0)
    {
        ESP_LOGI(__FUNCTION__, "set toc do quat cho dieu hoa");
        int mode = 0;
        char *valueString = dataPayload.metrics[0].value.string_value;
        if (strcmp(valueString, "auto") == 0)
        {
            mode = FAN_AUTO;
        }
        else if (strcmp(valueString, "speed_low") == 0)
        {
            mode = FAN_LOW;
        }
        else if (strcmp(valueString, "speed_medium") == 0)
        {
            mode = FAN_MEDIUM;
        }
        else if (strcmp(valueString, "speed_high") == 0)
        {
            mode = FAN_HIGH;
        }

        // todo push data to thangBoard
        handleFanModeAirCoodinatorCommand(device, mode);
        // handle publsh thingsboard
        handleAirCoodinatorCurrentFanSpeedDeviceData(dataInput.deviceId,
                                                     valueString);
    }

    // handle command temporary
    else if (strcmp(dataPayload.metrics[0].name,
                    "states/thermostatTemperatureSetpoint") == 0 ||
             strcmp(dataPayload.metrics[0].name,
                    "commands/ThermostatTemperatureSetpoint/"
                    "thermostatTemperatureSetpoint") == 0)
    {
        ESP_LOGI(__FUNCTION__, "set nhiet do cho dieu hoa");
        float value = dataPayload.metrics[0].value.float_value;
        // to do push data set to stm32
        int temp = value / 1;
        handleSetTempModeAirCoodinatorCommand(device, temp);
        // // handle publsh thingsboard
        handleAirCoodinatorSetTempDeviceData(dataInput.deviceId, value);
    }
    free_payload(&dataPayload);
}

/// @brief  handle set on off air and publish data to thingboard
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void handleSetOnOffAirCoodinatorCommand(attributeDevice_t device, int states)
{
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t reg = 0xFFFF;
    if (device.channel == 1)
    {
        reg = POWER_AIRCOODINATOR1_REGISTER;
    }
    else
    {
        reg = POWER_AIRCOODINATOR2_REGISTER;
    }
    // parse address output
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)reg << 8 | 1;
    pushDataToQueueOutput(device.type, address, states);
    latchControlAircoodinator(device.channel);
}

/// @brief  handle set mode for Aircoodinator
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void handleActiveModeAirCoodinatorCommand(attributeDevice_t device, int mode)
{
    // todo push to queue output
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t reg = 0xFFFF;
    if (device.channel == 1)
    {
        reg = MODE_AIRCOODINATOR1_REGISTER;
    }
    else
    {
        reg = MODE_AIRCOODINATOR2_REGISTER;
    }
    // parse address output
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)reg << 8 | 1;
    pushDataToQueueOutput(device.type, address, mode);
    latchControlAircoodinator(device.channel);
}

/// @brief handle set fan mode aircoordinator
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void handleFanModeAirCoodinatorCommand(attributeDevice_t device, int mode)
{
    // todo push to queue output
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t reg = 0xFFFF;
    if (device.channel == 1)
    {
        reg = FAN_AIRCOODINATOR1_REGISTER;
    }
    else
    {
        reg = FAN_AIRCOODINATOR2_REGISTER;
    }
    // parse address output
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)reg << 8 | 1;
    pushDataToQueueOutput(device.type, address, mode);
    latchControlAircoodinator(device.channel);
}

/// @brief  Set Temp Air
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void handleSetTempModeAirCoodinatorCommand(attributeDevice_t device, int temp)
{
    // todo push to queue output
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t reg = 0xFFFF;
    if (device.channel == 1)
    {
        reg = TEMP_AIRCOODINATOR1_REGISTER;
    }
    else
    {
        reg = TEMP_AIRCOODINATOR2_REGISTER;
    }
    // parse address output
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)reg << 8 | 1;
    pushDataToQueueOutput(device.type, address, temp);
    latchControlAircoodinator(device.channel);
}

/// @brief latch control after set config for aircoordinator
/// @param channel: channel need control
void latchControlAircoodinator(uint8_t channel)
{
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t reg = getRegisterfromChannel(channel);
    uint8_t value = 1;
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)reg << 8 | 1;
    pushDataToQueueOutput(airCoodinatorIR, address, value);
}

/// @brief get register control Air same channel control
/// @param channel: Channel neeed set
/// @return Register need control
uint16_t getRegisterfromChannel(uint8_t channel)
{
    switch (channel)
    {
        case 1:
            return LATCH_AIRCOODINATOR1_REGISTER;
        case 2:
            return LATCH_AIRCOODINATOR2_REGISTER;
        default:
            return 0xFFFF;
    }
}

/// @brief Function control dimmer
/// @param device:  include attributes of device need control
/// @param dataInput: Data topic, payload, device id,..  packge mqtt
void parsePayloadDimmer(attributeDevice_t device, sMqttPkg_t dataInput)
{
    uint16_t addr = RCU_MODBUS_ADDRESS_DEFAULT;
    uint16_t regDim = 0;
    uint8_t value = 0;
    uint8_t *buf = (uint8_t *)dataInput.strPayload;
    org_eclipse_tahu_protobuf_Payload dataPayload =
        org_eclipse_tahu_protobuf_Payload_init_zero;
    if (decode_payload(&dataPayload, buf, dataInput.lenPayload) < 0)
    {
        fprintf(stderr, "Failed to decode the payload\n");
        free_payload(&dataPayload);
        return;
    }
    if (dataPayload.metrics_count != 1)
    {
        free_payload(&dataPayload);
        return;
    }
    if ((strcmp(dataPayload.metrics[0].name, "states/brightness") != 0) &&
        (strcmp(dataPayload.metrics[0].name,
                "commands/BrightnessAbsolute/brightness") != 0))
    {
        free_payload(&dataPayload);
        return;
    }
    float percent = dataPayload.metrics[0].value.float_value;
    // Control channel Dimmer
    switch (device.channel)
    {
        case 1:
        {
            regDim = DIM1_REGISTER_DEFAULT;
            break;
        }
        case 2:
        {
            regDim = DIM2_REGISTER_DEFAULT;
            break;
        }
        default:
        {
            ESP_LOGE(__FUNCTION__, "NOT SUPPORT CHANNEL");
            return;
        }
    }
    if (percent < 0 || percent > 100)
    {
        percent > 0 ? (value = 100) : (value = 0);
    }
    else
    {
        value = percent;
    }
    uint32_t address = (uint32_t)addr << 24 | (uint32_t)regDim << 8;
    pushDataToQueueOutput(device.type, address, value);
    free_payload(&dataPayload);
    // thuc hien publish nguoc lai mqtt
    handleDimmerDeviceData(dataInput.deviceId, percent);
    free_payload(&dataPayload);
}
