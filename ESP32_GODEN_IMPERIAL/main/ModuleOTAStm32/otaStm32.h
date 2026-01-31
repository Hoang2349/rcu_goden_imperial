/*
 *      Author: thanghd
 */
/*
 * WRITE
 */

#ifndef __OTA_STM32_H__
#define __OTA_STM32_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STM_ACK (0x79)
#define STM_NACK (0x1F)
#define TIME_CHECK_ACK (100)  // timeOut ms
#define TIME_WRITE_FLASH_ACK (100)
#define TIME_WRITE_EARSE_FLASH (500)

#define CODE_FUNCTION_BOOTLOADER (0x7F)
#define CODE_FUNCTION_EXIT_BOOTLOADER (0xB14E)
#define CODE_FUNCTION_MODE_ERASE_FLASH (0X44BB)
#define CODE_FUNCTION_ERASE_APP_FLASH (0xFFFF)
#define CODE_FUNCTION_WIRTE_FLASH (0x31CE)
#define CODE_FUNCTION_GET_VERSION (0XB24D)
#define CODE_FUNCTION_WRITE_VERSION (0xB34C)
#define CODE_FUNCTION_READ_MEMORY (0x11EE)
#define CODE_FUNCTION_KEEP_ALIVE_STM32 (0xB44B)
#define CODE_FUNCTION_GET_CRC32 (0xB54A)
#define maxLengthVersion (8)

#define FUNCTION_CODE_GET_VERSION (0X03)
#define FUNCTION_CODE_START_BOOT (0X06)

#define ADDRESS_CHECK_STATUS_STM32 (0X0014)
#define ADDRESS_START_GET_VERSION (0X012)
#define ADDRESS_START_BOOT (0X0011)
#define VALUE_SET_MODE (0X007F)
#define CHUNK_SIZE_OTA (256)

#define START_ADDRESS_OTA_1 (0X08)
#define START_ADDRESS_OTA_2 (0X00)
#define START_ADDRESS_OTA_3 (0X60)
#define START_ADDRESS_OTA_4 (0X00)
enum statusCodeStm32
{
    STM32_OK = 0,
};

#ifdef __cplusplus
extern "C"
{
#endif

    void stm32OTAInit();
    void otaSendRequest(const char *data, int len);
    bool otaCheckAck(int timeOut);

    bool cmdStartBootStm32();
    bool cmdExitBootStm32();
    bool cmdEraseFlashStm32();
    bool cmdWriteVersionStm32();

    bool eraseFlashMemoryAppStm32();
    bool cmdWriteFlashStm32();
    bool setRegisterToAppFlash(const char *data);
    bool writeDataToRegistorFlash(const uint32_t dataFlash);
    bool writeNewVersionStm32(const char *version);
    bool getVersionStm32(char *ver);
    char xorByteArray(const char *byteArray, int length);
    void flashStm32viaSpiffs(const char *file_name, char *newVersion);
    bool flashPage(const char *address, const char *data);
    void incrementLoadAddress(char *loadAddr);
    bool writeFlashStm32(char *address, const char *data, int sizeFlash);
    bool getCrcStm32Flash(uint32_t *checksum);
    bool getStatusStm32(uint8_t *evenId);
    void timerStatusStm32();
    // void flashStm32(const char *newVersion,const char *file_name);
#ifdef __cplusplus
}
#endif

#endif /*__STM32OTA_H_*/
