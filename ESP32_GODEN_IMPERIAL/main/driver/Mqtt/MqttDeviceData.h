#ifndef __MQTT_DEVICE_DATA_H_
#define __MQTT_DEVICE_DATA_H_

#include <stdbool.h>
#include <stdint.h>

#define MAX_LENGTH_DEVICE_ID (20)
#define MAX_LENGTH_STATE_MOTION (50)
enum statesDoor
{
    OCCUPIED = 1,
    UNOCCUPIED = 0
};

#ifdef __cplusplus
extern "C"
{
#endif
    void handldeSwitchDeviceData(uint8_t address, uint8_t pin, uint8_t value);
    void handldeMotionDeviceData(uint8_t address, uint8_t pin, uint8_t value);
    void handldeSwitchRs485DeviceData(uint8_t address, uint8_t pin,
                                      uint8_t value);
    void handldeRelayDeviceData(int deviceId, int value);
    void handleCurtainDeviceData(int deviceId, const char *valueString);

    void handleAirCoodinatorCurrentFanSpeedDeviceData(int deviceId,
                                                      const char *fanMode);
    void handleAirCoodinatorStatusDeviceData(int deviceId, int states);
    void handleAirCoodinatorSetTempDeviceData(int deviceId, float tempSetPoint);
    void handleAirCoodinatorActiveModeDeviceData(int deviceId,
                                                 const char *activeMode);

    void handleDoorDeviceData(uint8_t address, uint8_t pin, uint8_t value);

    void handleDimmerDeviceData(int deviceId, float percent);
    void pushInforHeap(uint32_t free);
#ifdef __cplusplus
}
#endif

#endif
