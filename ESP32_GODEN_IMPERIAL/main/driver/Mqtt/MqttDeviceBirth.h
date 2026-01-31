#ifndef __MQTT_DEVICE_BIRTH_H_
#define __MQTT_DEVICE_BIRTH_H_

#define MAX_LENGTH_DEVICE_ID (10)
#define TEMP_SET_DEFAULT (26.5)


#ifdef __cplusplus
extern "C"
{
#endif
    void handleSwitchDeviceBirth(int deviceId, const char *nameType);
    void handleOutputDeviceBirth(int deviceId, const char *nameType);
    void handleMotionDryDeviceBirth(int deviceId, const char *nameType);
    void handleDoorDryDeviceBirth(int deviceId, const char *nameType);
    void handleRelayCurtainDeviceBirth(int deviceId, const char *nameType);
    void handleAirCoodinatorIRDeviceBirth(int deviceId, const char *nameType);
    void handleDimmerDeviceBirth(int deviceId, const char *nameType);
    void deviceBirth(char *jsonObject);
#ifdef __cplusplus
}
#endif

#endif