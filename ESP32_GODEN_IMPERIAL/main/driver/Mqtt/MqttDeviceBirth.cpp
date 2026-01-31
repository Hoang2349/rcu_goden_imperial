#include "MqttDeviceBirth.h"

#include <stdio.h>

#include "MapRuleMqtt.h"
#include "Mqtt.h"
#include "cJSON.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "tahu.h"
#include "tahu_pb.h"

/// @brief parse Device Birth and publish to server
/// @param jsonObject: point to string data of device need to debirth
void deviceBirth(char *jsonObject)
{
    cJSON *root = cJSON_Parse(jsonObject);
    if (root == NULL)
    {
        ESP_LOGE(__FUNCTION__, "json parse false");
        return;
    }
    ESP_LOGI(__FUNCTION__, "Success");
    // parse attribute on item
    cJSON *endpointId = cJSON_GetObjectItem(root, "endpointId");
    cJSON *hardwareInfo = cJSON_GetObjectItem(root, "hardwareInfo");
    cJSON *sparkplugMetrics = cJSON_GetObjectItem(root, "sparkplugMetrics");
    // parse decvice-id
    int deviceId = endpointId->valueint;
    // parse hardware information
    cJSON *deviceType = cJSON_GetObjectItem(hardwareInfo, "deviceType");
    char *type = deviceType->valuestring;
    // get Typehandle to Map
    int typeHandle = getStringToDeviceType(type);
    ESP_LOGW(__FUNCTION__, "Data Type handle: %d", typeHandle);
    switch (typeHandle)
    {
        case switchRj45:
            // handle Switch rj45 device birth
            {
                cJSON *pinNumber =
                    cJSON_GetObjectItem(hardwareInfo, "pinNumber");
                int pin = pinNumber->valueint;
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleSwitchDeviceBirth(deviceId, nameDevice);
                // insert to map switch rj45
                uint16_t key = (RCU_MODBUS_ADDRESS_DEFAULT << 8) | pin;
                insertSwitchPintoId(key, deviceId);
                insertRcuInputToType(key, switchRj45);
                // publish device birth
            }
            break;
        case outputRj45:
            // handle Switch rj45 device birth
            {
                cJSON *outputNumber =
                    cJSON_GetObjectItem(hardwareInfo, "outputNumber");
                int outputPin = outputNumber->valueint;
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleOutputDeviceBirth(deviceId, nameDevice);
                // insert map
                attributeDevice_t device = {0};
                device.type = outputRj45;
                device.address = RCU_MODBUS_ADDRESS_DEFAULT;
                device.pin = outputNumber->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;
        case relay:
            // handle Switch rj45 device birth
            {
                cJSON *relayNumber =
                    cJSON_GetObjectItem(hardwareInfo, "relayNumber");
                int relayPin = relayNumber->valueint;
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleOutputDeviceBirth(deviceId, nameDevice);
                // insert data to map
                attributeDevice_t device = {0};
                device.type = relay;
                device.address = RCU_MODBUS_ADDRESS_DEFAULT;
                device.pin = relayNumber->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;
        case motionRj45:
            // handle motion sensor input rj45
            {
                cJSON *pinNumber =
                    cJSON_GetObjectItem(hardwareInfo, "pinNumber");
                int pin = pinNumber->valueint;
                cJSON *triggerLevel =
                    cJSON_GetObjectItem(hardwareInfo, "triggerLevel");
                char *trigger = triggerLevel->valuestring;

                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleMotionDryDeviceBirth(deviceId, nameDevice);

                // insert to map sensor rj45
                uint16_t key = (RCU_MODBUS_ADDRESS_DEFAULT << 8) | pin;
                insertSwitchPintoId(key, deviceId);
                insertRcuInputToType(key, motionRj45);
            }
            break;
        case doorRj45:
            // handle motion sensor input rj45
            {
                attributeSensor_t sensor;
                cJSON *pinNumber =
                    cJSON_GetObjectItem(hardwareInfo, "pinNumber");
                int pin = pinNumber->valueint;
                cJSON *triggerLevel =
                    cJSON_GetObjectItem(hardwareInfo, "triggerLevel");
                char *trigger = triggerLevel->valuestring;

                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleDoorDryDeviceBirth(deviceId, nameDevice);
                // insert to map sensor rj45
                uint16_t key = (RCU_MODBUS_ADDRESS_DEFAULT << 8) | pin;
                insertSwitchPintoId(key, deviceId);
                insertRcuInputToType(key, doorRj45);
                // publish device birth
                sensor.type = doorRj45;
                sensor.address = RCU_MODBUS_ADDRESS_DEFAULT;
                sensor.pin = pin;
                if (strcmp(trigger, "PULL-UP") == 0)
                {
                    sensor.triggerLevel = levelUp;
                }
                else
                {
                    sensor.triggerLevel = levelDown;
                }
                insertDeviceIdToAttributeSensor(key, sensor);
            }
            break;
        case switchModbus:
            // handle motion sensor input rj45
            {
                cJSON *pinNumber =
                    cJSON_GetObjectItem(hardwareInfo, "pinNumber");
                int pin = pinNumber->valueint;
                cJSON *addressModbus =
                    cJSON_GetObjectItem(hardwareInfo, "addressModbus");
                uint8_t address = addressModbus->valueint;

                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleSwitchDeviceBirth(deviceId, nameDevice);

                // insert to map switch rj45
                uint16_t key = ((uint16_t)address << 8) | pin;
                insertSwitchPintoId(key, deviceId);
            }
            break;
        case ledModbus:
            // handle motion sensor input rj45
            {
                cJSON *ledNumber =
                    cJSON_GetObjectItem(hardwareInfo, "ledNumber");
                int pin = ledNumber->valueint;
                cJSON *addressModbus =
                    cJSON_GetObjectItem(hardwareInfo, "addressModbus");
                int address = addressModbus->valueint;

                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                // publish device birth
                handleOutputDeviceBirth(deviceId, nameDevice);
                // insert map
                attributeDevice_t device = {0};
                device.type = ledModbus;
                device.address = address;
                device.pin = pin;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;
        case curtainRelay:
            // handlde Curstain
            {
                cJSON *openPin = cJSON_GetObjectItem(hardwareInfo, "openPin");
                cJSON *closePin = cJSON_GetObjectItem(hardwareInfo, "closePin");
                cJSON *timeOut = cJSON_GetObjectItem(hardwareInfo, "timeOut");
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;

                handleRelayCurtainDeviceBirth(deviceId, nameDevice);
                attributeDevice_t device = {0};
                device.type = curtainRelay;
                device.pinOpen = openPin->valueint;
                device.pinClose = closePin->valueint;
                device.timeOut = timeOut->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;
        case curtainRj45:
            // handlde Curstain
            {
                cJSON *openPin = cJSON_GetObjectItem(hardwareInfo, "openPin");
                cJSON *closePin = cJSON_GetObjectItem(hardwareInfo, "closePin");
                cJSON *timeOut = cJSON_GetObjectItem(hardwareInfo, "timeOut");
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;

                handleRelayCurtainDeviceBirth(deviceId, nameDevice);
                attributeDevice_t device = {0};
                device.type = curtainRj45;
                device.pinOpen = openPin->valueint;
                device.pinClose = closePin->valueint;
                device.timeOut = timeOut->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;

        case airCoodinatorIR:
            // handle AirCoodinator
            {
                cJSON *channel = cJSON_GetObjectItem(hardwareInfo, "channel");
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                handleAirCoodinatorIRDeviceBirth(deviceId, nameDevice);
                attributeDevice_t device = {0};
                device.type = airCoodinatorIR;
                device.channel = channel->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;
        case dimmer:
            // handle Dimmer onboard RCU
            {
                cJSON *channel = cJSON_GetObjectItem(hardwareInfo, "channel");
                cJSON *value = cJSON_GetObjectItem(sparkplugMetrics, "value");
                char *nameDevice = value->valuestring;
                handleDimmerDeviceBirth(deviceId, nameDevice);
                attributeDevice_t device = {0};
                device.type = dimmer;
                device.channel = channel->valueint;
                insertDeviceIdToAttribute(deviceId, device);
            }
            break;

        default:
            ESP_LOGE(__FUNCTION__, "Not Handle DeviceType");
            break;
    }
    cJSON_Delete(root);
}

// handle switch rj45 debith and modbus

/// @brief handle switch rj45 debith and modbus, it creat metrics for device
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleSwitchDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;
    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);

    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    bool newValue = false;
    add_simple_metric(&dataPayload, "attributes/commandOnlyOnOff", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    newValue = true;
    add_simple_metric(&dataPayload, "attributes/queryOnlyOnOff", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    newValue = false;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));

    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    cpyArray(payload, (char *)binaryBuffer, 0, payloadLen);
    free_payload(&dataPayload);
    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

/// @brief handle Outprut Rj45 and relay devbirth
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleOutputDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;
    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    // printf("topic: %s\n", topic);
    // parse payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    bool newValue = true;
    add_simple_metric(&dataPayload, "attributes/commandOnlyOnOff", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    newValue = false;
    add_simple_metric(&dataPayload, "attributes/queryOnlyOnOff", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    newValue = false;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    newValue = false;
    add_simple_metric(&dataPayload, "commands/OnOff/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &newValue,
                      sizeof(newValue));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);
    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

/// @brief parse data metrics device birth for sensor drycontact
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleMotionDryDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;

    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    char *typeSensor = "PHYSICAL_CONTACT";
    add_simple_metric(
        &dataPayload,
        "attributes/occupancySensorConfiguration/occupancySensorType", false, 0,
        METRIC_DATA_TYPE_STRING, false, false, typeSensor, strlen(typeSensor));
    uint8_t delaySec = 2;
    add_simple_metric(
        &dataPayload,
        "attributes/occupancySensorConfiguration/occupiedToUnoccupiedDelaySec",
        false, 0, METRIC_DATA_TYPE_UINT8, false, false, &delaySec,
        sizeof(delaySec));
    delaySec = 2;
    add_simple_metric(
        &dataPayload,
        "attributes/occupancySensorConfiguration/unoccupiedToOccupiedDelaySec",
        false, 0, METRIC_DATA_TYPE_UINT8, false, false, &delaySec,
        sizeof(delaySec));
    uint8_t eventThreshold = 2;
    add_simple_metric(&dataPayload,
                      "attributes/occupancySensorConfiguration/"
                      "unoccupiedToOccupiedEventThreshold",
                      false, 0, METRIC_DATA_TYPE_UINT8, false, false,
                      &eventThreshold, sizeof(eventThreshold));
    char *state = "UNOCCUPIED";
    add_simple_metric(&dataPayload, "states/occupancy", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, state,
                      strlen(state));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);

    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

//
/// @brief handle curtain ddebirth
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleRelayCurtainDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;

    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    // handle payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    char *valueString = "UP,DOWN";
    add_simple_metric(&dataPayload, "attributes/openDirection", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, valueString,
                      strlen(valueString));
    bool valueBool = true;
    add_simple_metric(&dataPayload, "attributes/pausable", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &valueBool,
                      sizeof(valueBool));
    char *valueString1 = "UP";
    add_simple_metric(&dataPayload, "states/openState/openDirection", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, valueString1,
                      strlen(valueString1));
    add_simple_metric(&dataPayload, "states/isRunning", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &valueBool,
                      sizeof(valueBool));
    add_simple_metric(&dataPayload, "commands/OpenClose/openDirection", false,
                      0, METRIC_DATA_TYPE_STRING, false, false, valueString1,
                      strlen(valueString1));
    add_simple_metric(&dataPayload, "commands/PauseUnpause/pause", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &valueBool,
                      sizeof(valueBool));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);

    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

/// @brief handle aircoodinator device birth'
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleAirCoodinatorIRDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH * 2];
    int payloadLen = 0;

    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    // handle payload
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    char *fanAttribute = "auto, speed_low, speed_medium, speed_high";
    add_simple_metric(&dataPayload, "attributes/availableFanSpeeds", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, fanAttribute,
                      strlen(fanAttribute));
    char *fanActive = "speed_low";
    add_simple_metric(&dataPayload, "states/currentFanSpeedSetting", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, fanActive,
                      strlen(fanActive));
    char *commandFanActive = "speed_low";
    add_simple_metric(&dataPayload, "commands/SetFanSpeed/fanSpeed", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, commandFanActive,
                      strlen(commandFanActive));
    char *attributeThemostatModes = "auto, heat, cool, dry, fan-only";
    add_simple_metric(&dataPayload, "attributes/availableThermostatModes",
                      false, 0, METRIC_DATA_TYPE_STRING, false, false,
                      attributeThemostatModes, strlen(attributeThemostatModes));
    char *tempUnit = "C";
    add_simple_metric(&dataPayload, "attributes/thermostatTemperatureUnit",
                      false, 0, METRIC_DATA_TYPE_STRING, false, false, tempUnit,
                      strlen(tempUnit));
    char *themostatMode = "heat";
    add_simple_metric(&dataPayload, "states/activeThermostatMode", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, themostatMode,
                      strlen(themostatMode));
    float tempSet = TEMP_SET_DEFAULT;
    add_simple_metric(&dataPayload, "states/thermostatTemperatureSetpoint",
                      false, 0, METRIC_DATA_TYPE_FLOAT, false, false, &tempSet,
                      sizeof(tempSet));
    tempSet = TEMP_SET_DEFAULT;
    add_simple_metric(&dataPayload, "states/thermostatTemperatureAmbient",
                      false, 0, METRIC_DATA_TYPE_FLOAT, false, false, &tempSet,
                      sizeof(tempSet));
    bool state = true;
    add_simple_metric(&dataPayload, "states/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &state,
                      sizeof(state));
    state = true;
    add_simple_metric(&dataPayload, "commands/OnOff/on", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &state,
                      sizeof(state));
    char *commandActiveMode = "cool";
    add_simple_metric(&dataPayload, "commands/ThermostatSetMode/thermostatMode",
                      false, 0, METRIC_DATA_TYPE_STRING, false, false,
                      commandActiveMode, strlen(commandActiveMode));
    float commandSetTemp = TEMP_SET_DEFAULT;
    add_simple_metric(
        &dataPayload,
        "commands/ThermostatTemperatureSetpoint/thermostatTemperatureSetpoint",
        false, 0, METRIC_DATA_TYPE_FLOAT, false, false, &commandSetTemp,
        sizeof(commandSetTemp));

    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH * 2;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);

    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

/// @brief handle debirth door sensor RJ45
/// @param deviceId: this is device ID of Device
/// @param nameType: name of type device
void handleDoorDryDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;

    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    bool valueState = true;
    add_simple_metric(&dataPayload, "attributes/queryOnlyOpenState", false, 0,
                      METRIC_DATA_TYPE_BOOLEAN, false, false, &valueState,
                      sizeof(valueState));
    int intState = 0;
    add_simple_metric(&dataPayload, "states/openState/openPercent", false, 0,
                      METRIC_DATA_TYPE_UINT8, false, false, &intState,
                      sizeof(intState));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);
    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}

/// @brief hanlde DeBirth Dimmmer onboard RCU
/// @param deviceId: this is device ID of Device 
/// @param nameType: name of type device 
void handleDimmerDeviceBirth(int deviceId, const char *nameType)
{
    char topic[MQTT_MAXIMUM_TOPIC_LENGTH];
    char payload[MQTT_MAXIMUM_MESSAGE_LENGTH];
    int payloadLen = 0;

    // parse topic
    char stringDeviceId[MAX_LENGTH_DEVICE_ID];
    snprintf(stringDeviceId, sizeof(stringDeviceId), "%d", deviceId);
    strcpy(topic, TOPIC_EXAMPLE_DBIRTH);
    strcat(topic, stringDeviceId);
    org_eclipse_tahu_protobuf_Payload dataPayload;
    get_next_payload(&dataPayload);
    add_simple_metric(&dataPayload, "attributes/deviceType", false, 0,
                      METRIC_DATA_TYPE_STRING, false, false, nameType,
                      strlen(nameType));
    bool valueState = true;
    add_simple_metric(&dataPayload, "attributes/commandOnlyBrightness", false,
                      0, METRIC_DATA_TYPE_BOOLEAN, false, false, &valueState,
                      sizeof(valueState));
    float percent = 0;
    add_simple_metric(&dataPayload, "states/brightness", false, 0,
                      METRIC_DATA_TYPE_FLOAT, false, false, &percent,
                      sizeof(percent));
    add_simple_metric(&dataPayload, "commands/BrightnessAbsolute/brightness",
                      false, 0, METRIC_DATA_TYPE_FLOAT, false, false, &percent,
                      sizeof(percent));
    size_t bufferLength = MQTT_MAXIMUM_MESSAGE_LENGTH;
    uint8_t binaryBuffer[bufferLength]= {0};
    payloadLen = encode_payload(binaryBuffer, bufferLength, &dataPayload);
    char *str = (char *)binaryBuffer;
    cpyArray(payload, str, 0, payloadLen);
    free_payload(&dataPayload);

    // publish mqtt
    appMqttPublish(topic, payload, payloadLen);
}