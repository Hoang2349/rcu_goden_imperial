#ifndef __mapRuleMqttInit_H__
#define __mapRuleMqttInit_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RCU_MODBUS_ADDRESS_DEFAULT (0x09)

#define RELAY_REGISTER_DEFAULT (0x02)
#define RJ45_REGISTER1_DEFAULT (0x03)
#define RJ45_REGISTER2_DEFAULT (0x04)
#define LED_MODBUS_REGISTER_DEFAULT (0x1008)
#define DIM1_REGISTER_DEFAULT (0x0005)
#define DIM2_REGISTER_DEFAULT (0x0006)

#define POWER_AIRCOODINATOR1_REGISTER (0x0007)
#define TEMP_AIRCOODINATOR1_REGISTER (0x0008)
#define MODE_AIRCOODINATOR1_REGISTER (0x0009)
#define FAN_AIRCOODINATOR1_REGISTER (0x000A)
#define LATCH_AIRCOODINATOR1_REGISTER (0X000B)

#define POWER_AIRCOODINATOR2_REGISTER (0x000C)
#define TEMP_AIRCOODINATOR2_REGISTER (0x000D)
#define MODE_AIRCOODINATOR2_REGISTER (0x000E)
#define FAN_AIRCOODINATOR2_REGISTER (0x000F)
#define LATCH_AIRCOODINATOR2_REGISTER (0X0010)

enum typeDevice
{
    switchRj45 = 1,
    outputRj45 = 2,
    relay = 3,
    curtainRelay = 4,
    curtainRj45 = 5,
    motionRj45 = 6,
    doorRj45 = 7,
    switchModbus = 8,
    ledModbus = 9,
    airCoodinatorIR = 10,
    dimmer = 11,
};
enum LevelTrigger
{
    levelUp = 0,
    levelDown = 1,
};

typedef struct attributeOnOffstate
{
    uint8_t type;
    uint8_t address;
    uint8_t pin;
    uint8_t pinOpen;
    uint8_t pinClose;
    uint16_t timeOut;
    uint8_t channel;
} attributeDevice_t;

typedef struct attributeSensor
{
    uint8_t type;
    uint8_t address;
    uint8_t pin;
    uint8_t triggerLevel;
} attributeSensor_t;

enum attributeFanMode
{
    FAN_AUTO = 0,
    FAN_LOW = 1,
    FAN_MEDIUM = 2,
    FAN_HIGH = 3,
};

enum attributeActiveMode
{
    ACTIVE_AUTO = 0,
    ACTIVE_COOL = 1,
    ACTIVE_DRY = 2,
    ACTIVE_FAN_ONLY = 3,
    ACTIVE_HEAT = 4,
};

#ifdef __cplusplus
extern "C"
{
#endif
    void insertSwitchPintoId(int key, int value);
    int getSwitchPintoId(int key);

    void insertRcuInputToType(int key, int value);
    int getRcuInputToType(int key);

    void insertStringToDeviceType(char *key, int value);
    int getStringToDeviceType(char *key);

    void insertDeviceIdToAttribute(int key, attributeDevice_t value);
    attributeDevice_t getDeviceIdToAttribute(int key);

    void insertDeviceIdToAttributeSensor(int key, attributeSensor_t value);
    attributeSensor_t getDeviceIdToAttributeSensor(int key);
    void mapRuleMqttInit();
#ifdef __cplusplus
}
#endif

#endif