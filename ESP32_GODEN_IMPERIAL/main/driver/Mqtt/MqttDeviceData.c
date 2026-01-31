#include "MqttDeviceData.h"

#include <stdbool.h>

#include "MapRuleMqtt.h"
#include "Mqtt.h"
#include "esp_log.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tahu.h"
#include "tahu_pb.h"

/// @brief handle switch publish data to mqtt
/// @param address: slave address
/// @param pin: Pin number need publish
/// @param value: Value publish
void handldeSwitchDeviceData(uint8_t address, uint8_t pin, uint8_t value)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // get device id mapping with switch
    uint16_t key = ((uint16_t)address << 8) | pin;
    ESP_LOGI(__FUNCTION__, "KEY : 0x%X", key);
    int deviceId = getSwitchPintoId((int)key);
    if (deviceId == 0)
    {
        // printf("deviceid : %d\n", deviceId);
        return;
    }
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    // bool new_value = true;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &value,
                      sizeof(value));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief handle motion sensor publish data
/// @param address: slave address
/// @param pin: Pin number need publish
/// @param value: Value publish
void handldeMotionDeviceData(uint8_t address, uint8_t pin, uint8_t value)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // get device id mapping with switch
    uint16_t key = ((uint16_t)address << 8) | pin;
    ESP_LOGI(__FUNCTION__, "KEY : 0x%X", key);
    int deviceId = getSwitchPintoId((int)key);
    if (deviceId == 0)
    {
        // printf("deviceid : %d\n", deviceId);
        return;
    }
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    // check status input
    char state[MAX_LENGTH_STATE_MOTION] = {0};
    if (value == OCCUPIED)
    {
        strcpy(state, "OCCUPIED");
    }
    else if (value == UNOCCUPIED)
    {
        strcpy(state, "UNOCCUPIED");
    }
    else
    {
        strcpy(state, "UNKNOWN_OCCUPANCY_STATE");
    }

    add_simple_metric(&dataPayload, "states/occupancy", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, &state,
                      sizeof(state));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief handle switch modbus rs485
/// @param address: slave address
/// @param pin: Pin number need publish
/// @param value: Value publish
void handldeSwitchRs485DeviceData(uint8_t address, uint8_t pin, uint8_t value)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // get device id mapping with switch
    uint16_t key = ((uint16_t)address << 8) | pin;
    ESP_LOGI(__FUNCTION__, "KEY : 0x%X", key);
    int deviceId = getSwitchPintoId((int)key);
    if (deviceId == 0)
    {
        // printf("deviceid : %d\n", deviceId);
        return;
    }
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    // bool new_value = true;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &value,
                      sizeof(value));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief push relay sates on thingsboard
/// @param address: slave address
/// @param value: Value publish
void handldeRelayDeviceData(int deviceId, int value)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    // bool new_value = true;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &value,
                      sizeof(value));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief publish Curtain states on thingsboard
/// @param deviceId: Device id of device
/// @param valueString : states curtain "OPEN" or "CLOSE"
void handleCurtainDeviceData(int deviceId, const char *valueString)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/openState/openDirection", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, valueString,
                      strlen(valueString));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief  Publish device data for Aircoodinator
/// @param deviceId: Device id of device
/// @param states : states on/off of Aircoordinator
void handleAirCoodinatorStatusDeviceData(int deviceId, int states)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &states,
                      sizeof(states));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Function publish speed current of Air
/// @param deviceId : deive Id of device
/// @param fanMode : mode fan high, medium, slow
void handleAirCoodinatorCurrentFanSpeedDeviceData(int deviceId,
                                                  const char *fanMode)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/currentFanSpeedSetting", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, fanMode,
                      strlen(fanMode));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Publish mode of Air
/// @param deviceId : deivce Id of device
/// @param activeMode : mode of Air heat, cool, fan-only and auto
void handleAirCoodinatorActiveModeDeviceData(int deviceId,
                                             const char *activeMode)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/activeThermostatMode", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, activeMode,
                      strlen(activeMode));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Function publish temp current Air
/// @param deviceId : device id of device
/// @param tempSetPoint : temp current of Air
void handleAirCoodinatorSetTempDeviceData(int deviceId, float tempSetPoint)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/thermostatTemperatureSetpoint",
                      false, 0, METRIC_DATA_TYPE_FLOAT, false, false,
                      &tempSetPoint, sizeof(tempSetPoint));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Publish state dooor to mqtt
/// @param address: slave address
/// @param pin: Pin number need publish
/// @param value: Value publish
void handleDoorDeviceData(uint8_t address, uint8_t pin, uint8_t value)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // get device id mapping with switch
    uint16_t key = ((uint16_t)address << 8) | pin;
    ESP_LOGI(__FUNCTION__, "KEY : 0x%X", key);
    int deviceId = getSwitchPintoId((int)key);
    if (deviceId == 0)
    {
        // printf("deviceid : %d\n", deviceId);
        return;
    }
    attributeSensor_t sensor = getDeviceIdToAttributeSensor(key);
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    // check status input
    uint8_t valueDoor = 0;
    if (value == 1)
    {
        sensor.triggerLevel == levelUp ? (valueDoor = 0) : (valueDoor = 100);
    }
    else if (value == 0)
    {
        sensor.triggerLevel == levelUp ? (valueDoor = 100) : (valueDoor = 0);
    }
    add_simple_metric(&dataPayload, "states/openState/openPercent", false, 0,
                      METRIC_DATA_TYPE_UINT8, false, false, &valueDoor,
                      sizeof(valueDoor));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Publish value Dim current to mqtt
/// @param deviceId : device Id of Device
/// @param percent : percent dim 0% -> 100%
void handleDimmerDeviceData(int deviceId, float percent)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, TOPIC_EXAMPLE_DDATA);
    char idTopic[MAX_LENGTH_DEVICE_ID] = {0};
    sprintf(idTopic, "%d", deviceId);
    strcat(topic, idTopic);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "states/brightness", false, 0,
                      METRIC_DATA_TYPE_FLOAT, false, false, &percent,
                      sizeof(percent));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}

/// @brief Function pbulish memory heap free to mqtt
/// @param value heap free
void pushInforHeap(uint32_t free)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    // Parse topic
    strcpy(topic, "spBv1.0/Sparkplug Group 1/NDATA/Edge Node");
    // Parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "HeapFree/System", false, 0,
                      METRIC_DATA_TYPE_UINT32, false, false, &free,
                      sizeof(free));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength];
    int len = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *payload = (char *)binaryBuffer;
    appMqttPublish(topic, payload, len);
    free_payload(&dataPayload);
}
