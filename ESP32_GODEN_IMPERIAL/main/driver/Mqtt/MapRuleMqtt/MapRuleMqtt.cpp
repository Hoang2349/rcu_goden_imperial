#include "MapRuleMqtt.h"

#include <iostream>
#include <map>
struct CharPtrComparer
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};

std::map<int, int> switchMapPintoId;
std::map<const char *, int, CharPtrComparer> deviceType;
std::map<int, int> inputMapPintoType;
std::map<int, attributeDevice_t> stateMapDeviceIdtoAttribute;
std::map<int, attributeSensor_t> sensorMapDeviceIdtoAttribute;

/// @brief Insert key value in to table
/// @param key : this data include add, reg and pin
/// @param value : Value of button
void insertSwitchPintoId(int key, int value) { switchMapPintoId[key] = value; }

/// @brief Get value from key
/// @param key :  this data include add, reg and pin
/// @return Value of key get
int getSwitchPintoId(int key)
{
    auto data = switchMapPintoId.find(key);
    if (data != switchMapPintoId.end())
    {
        return data->second;
    }
    return 0;
}

/// @brief String map to Type device
/// @param key : string need insert
/// @param value : type deive
void insertStringToDeviceType(char *key, int value) { deviceType[key] = value; }

/// @brief get Device type from key
/// @param key: string need get type
/// @return: return device type(int) if no handle return 0
int getStringToDeviceType(char *key)
{
    auto data = deviceType.find(key);
    if (data != deviceType.end())
    {
        return data->second;
    }
    return 0;
}

/// @brief Handle Map Switch 485
/// @param key : include addr, reg,and pin RJ45
/// @param value : value of key need save
void insertRcuInputToType(int key, int value)
{
    inputMapPintoType[key] = value;
}

/// @brief Get value of key
/// @param key : include addr, reg,and pin RJ45
/// @return value need get from key
int getRcuInputToType(int key)
{
    auto data = inputMapPintoType.find(key);
    if (data != inputMapPintoType.end())
    {
        return data->second;
    }
    return 0;
}

/// @brief insert device id and value corresponding value
/// @param key : device id
/// @param value : include atrisbutes of device id
void insertDeviceIdToAttribute(int key, attributeDevice_t value)
{
    stateMapDeviceIdtoAttribute[key] = value;
}

/// @brief Get attributes of deive id
/// @param key : Device Id device
/// @return attributes corresponding device id
attributeDevice_t getDeviceIdToAttribute(int key)
{
    auto data = stateMapDeviceIdtoAttribute.find(key);
    if (data != stateMapDeviceIdtoAttribute.end())
    {
        return data->second;
    }
    attributeDevice_t a = {0};
    return a;
}

/// @brief  insert deviceID to attribute sensor
/// @param key : deivice id
/// @param value : atribute need save
void insertDeviceIdToAttributeSensor(int key, attributeSensor_t value)
{
    sensorMapDeviceIdtoAttribute[key] = value;
}

/// @brief Get attributes from device id
/// @param key : device id
/// @return attributes sensor need get
attributeSensor_t getDeviceIdToAttributeSensor(int key)
{
    auto data = sensorMapDeviceIdtoAttribute.find(key);
    if (data != sensorMapDeviceIdtoAttribute.end())
    {
        return data->second;
    }
    attributeSensor_t a = {0};
    return a;
}

/// @brief Function init string key with type value
void mapRuleMqttInit()
{
    insertStringToDeviceType("switchRj45", switchRj45);
    insertStringToDeviceType("outputRj45", outputRj45);
    insertStringToDeviceType("relay", relay);
    insertStringToDeviceType("curtainRelay", curtainRelay);
    insertStringToDeviceType("curtainRj45", curtainRj45);
    insertStringToDeviceType("motionRj45", motionRj45);
    insertStringToDeviceType("doorRj45", doorRj45);
    insertStringToDeviceType("switchModbus", switchModbus);
    insertStringToDeviceType("ledModbus", ledModbus);
    insertStringToDeviceType("aircooler", airCoodinatorIR);
    insertStringToDeviceType("dimmer", dimmer);
}