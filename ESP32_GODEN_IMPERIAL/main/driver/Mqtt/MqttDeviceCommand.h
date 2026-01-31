#ifndef __MQTT_DEVICE_COMMAND_H_
#define __MQTT_DEVICE_COMMAND_H_

#include <stdbool.h>
#include <stdint.h>

#include "MapRuleMqtt.h"
#include "Mqtt.h"

#define MAX_NUMBER_PIN_PORT1 (16)
#define MAX_NUMBER_PIN_PORT2 (27)
#define DELTA_GET_BIT_CONTROL (15)
#define MAX_METRICS_HANDLE (1)
#define TIME_WAIT_TASK_CURTAIN (10 / portTICK_PERIOD_MS)

enum curtainStateRelay
{
    ACTIVE_RELAY = 1,
    UNACTIVE_RELAY = 0
};
#ifdef __cplusplus
extern "C"
{
#endif
    void initRcuCurtain();
    void parseDataMqttSparkplug(sMqttPkg_t dataInput);
    void parsePayloadRelayOnBoardDcmd(attributeDevice_t device,
                                      sMqttPkg_t dataInput);
    void parsePayloadRJ45OutputDcmd(attributeDevice_t device,
                                    sMqttPkg_t dataInput);
    void parsePayloadRS485LedDcmd(attributeDevice_t device,
                                  sMqttPkg_t dataInput);
    void parsePayloadCurtainDcmd(attributeDevice_t device,
                                 sMqttPkg_t dataInput);
    void parsePayloadDimmer(attributeDevice_t device, sMqttPkg_t dataInput);

    void parsePayloadAirCoodinatorIR(attributeDevice_t device,
                                     sMqttPkg_t dataInput);
    void handleSetOnOffAirCoodinatorCommand(attributeDevice_t device,
                                            int states);
    void handleActiveModeAirCoodinatorCommand(attributeDevice_t device,
                                              int mode);
    void handleFanModeAirCoodinatorCommand(attributeDevice_t device, int mode);
    void handleSetTempModeAirCoodinatorCommand(attributeDevice_t device,
                                               int mode);
    void latchControlAircoodinator(uint8_t channel);
    uint16_t getRegisterfromChannel(uint8_t channel);
#ifdef __cplusplus
}
#endif
#endif
