#include "otaStm32.h"

#include <stdio.h>

#include "AppSpiffs.h"
#include "Modbus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "uartInit.h"

#define GET_BYTE_U32T(value, n) (((value) >> (8 * (n))) & 0xFF)

/// @brief init Peripheral
void stm32OTAInit() { initUartOTA(); }

/*
    Funciton Send data via uart
    @ const char *data : con tro vi tri data can gui
    @ int len          : do dai data
*/
void otaSendRequest(const char *data, int len) { uartWrite(data, len); }

/*
    Function wait ACK from STM32 return
    @ int timeOut : thoi gian timeout = timeOut *100(ms)
    @ return    : true or false
*/
bool otaCheckAck(int timeOut)
{
    int lenData = 128;
    uint8_t timeCheck = 0;
    uint8_t data[lenData] = {0};

    int len = uartRead(data, lenData, timeOut);
    if (len != 0)
    {
        switch (data[0])
        {
            case STM_ACK:
            {
                return true;
            }
            case STM_NACK:
            {
                return false;
            }
            default:
            {
                break;
            }
        }
    }
    return false;
}

/*
    function call version STM32 using porrt Modbus RS485 connect to STm32
    @ char *ver : gia tri version tra ve
    @ return    : true or false
*/
bool getVersionStm32(char *ver)
{
    // Thuc hien giao tiep lay version bang RS485'
    int len = 8;
    int crc = 0;
    uint8_t quantity = 2;
    uint8_t buff[len] = {0};
    buff[0] = MODBUS_ADDRESS_RCU_DEFAULT;
    buff[1] = FUNCTION_CODE_GET_VERSION;
    buff[2] = HI_UINT16(ADDRESS_START_GET_VERSION);
    buff[3] = LO_UINT16(ADDRESS_START_GET_VERSION);
    buff[4] = HI_UINT16(quantity);
    buff[5] = LO_UINT16(quantity);
    crc = crc16(buff, len - 2);
    buff[6] = LO_UINT16((uint16_t)crc);
    buff[7] = HI_UINT16((uint16_t)crc);

    if (xSemaphoreTake(xMutexModbusPort, (TickType_t)1000) == pdTRUE)
    {
        modbusWrite((char *)buff, len);

        int lenData = 9;
        char data[lenData] = {0};

        if (!modbusRead((char *)data))
        {
            ESP_LOGE(__FUNCTION__, "1.Call version Erorr");
            xSemaphoreGive(xMutexModbusPort);
            return false;
        }
        crc = crc16((uint8_t *)data, lenData - 2);
        if (data[lenData - 1] != HI_UINT16(crc) ||
            data[lenData - 2] != LO_UINT16(crc))
        {
            ESP_LOGE(__FUNCTION__, "2.Call version Erorr");
            xSemaphoreGive(xMutexModbusPort);
            return false;
        }
        ver[0] = data[3];
        ver[1] = 0x2E;
        ver[2] = data[4];
        ver[3] = 0x2E;
        ver[4] = data[5];
        xSemaphoreGive(xMutexModbusPort);
        return true;
        // todo handle error
    }

    ESP_LOGE(__FUNCTION__, "Semaphore take error");
    return false;
}

/*
    Function goi vao che do boot tren STM32
*/
bool cmdStartBootStm32()
{
    int len = 8;
    int crc = 0;
    uint8_t buff[len] = {0};
    buff[0] = MODBUS_ADDRESS_RCU_DEFAULT;
    buff[1] = FUNCTION_CODE_START_BOOT;
    buff[2] = HI_UINT16(ADDRESS_START_BOOT);
    buff[3] = LO_UINT16(ADDRESS_START_BOOT);
    buff[4] = HI_UINT16(VALUE_SET_MODE);
    buff[5] = LO_UINT16(VALUE_SET_MODE);
    crc = crc16(buff, len - 2);
    buff[6] = LO_UINT16((uint16_t)crc);
    buff[7] = HI_UINT16((uint16_t)crc);
    if (xSemaphoreTake(xMutexModbusPort, (TickType_t)1000) == pdTRUE)
    {
        modbusWrite((char *)buff, len);

        int lenData = 8;
        char data[lenData] = {0};

        if (!modbusRead((char *)data))
        {
            ESP_LOGE(__FUNCTION__, "1.Call boot Erorr");
            xSemaphoreGive(xMutexModbusPort);
            return false;
        }
        crc = crc16((uint8_t *)data, lenData - 2);
        if (data[lenData - 1] != HI_UINT16(crc) ||
            data[lenData - 2] != LO_UINT16(crc))
        {
            ESP_LOGE(__FUNCTION__, "2.Call boot Erorr");
            xSemaphoreGive(xMutexModbusPort);
            return false;
        }
        xSemaphoreGive(xMutexModbusPort);
        return true;
        // todo handle error
    }

    ESP_LOGE(__FUNCTION__, "Semaphore take error");
    return false;
}

/*
    Thoat khoi che do boot cua STM32
*/
bool cmdExitBootStm32()
{
    // thoar khori che do boot
    int len = 2;
    char data[len] = {CODE_FUNCTION_EXIT_BOOTLOADER >> 8,
                      CODE_FUNCTION_EXIT_BOOTLOADER & 0x00FF};
    otaSendRequest(data, len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    ham vao che doa xoa bo nho STM32
*/
bool cmdEraseFlashStm32()
{
    int len = 2;
    char data[len] = {CODE_FUNCTION_MODE_ERASE_FLASH >> 8,
                      CODE_FUNCTION_MODE_ERASE_FLASH & 0x00FF};
    otaSendRequest(data, len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham thuc hien xoa vung nho applycation cua STM32
*/
bool eraseFlashMemoryAppStm32()
{
    // xoa bo nho Stm32 dtai vung nho chua chuong trinh
    int len = 3;
    char data[len] = {CODE_FUNCTION_ERASE_APP_FLASH >> 8,
                      CODE_FUNCTION_ERASE_APP_FLASH & 0x00FF, 0};
    data[len - 1] = xorByteArray(data, len - 1);
    otaSendRequest(data, len);
    // cho 500ms
    bool ret = otaCheckAck(TIME_WRITE_EARSE_FLASH);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham ghi vung nho flash STM32
*/
bool cmdWriteFlashStm32()
{
    // ghi du lieu vao bo nho STM32
    int len = 2;
    char data[len] = {CODE_FUNCTION_WIRTE_FLASH >> 8,
                      CODE_FUNCTION_WIRTE_FLASH & 0x00FF};
    otaSendRequest(&data[0], len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham set dia chi ghi du lieu tren STM32
    @ const char *address : dia chi can set
    @ return               : true or false
*/
bool setRegisterToAppFlash(const char *address)
{
    // ghi du lieu vao bo nho STM32
    int len = 5;
    char data[len] = {address[0], address[1], address[2], address[3], 0};
    data[len - 1] = xorByteArray(data, len - 1);
    otaSendRequest(data, len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham Ghi du lieu vao vung nho da set truoc do
    @ const uint32_t dataFlash : du lieu can ghi vao bo nho
    @ return                   : true or false
*/
bool writeDataToRegistorFlash(const uint32_t dataFlash)
{
    int len = 6;
    char data[len] = {0x03,
                      (uint8_t)GET_BYTE_U32T(dataFlash, 3),
                      (uint8_t)GET_BYTE_U32T(dataFlash, 2),
                      (uint8_t)GET_BYTE_U32T(dataFlash, 1),
                      (uint8_t)GET_BYTE_U32T(dataFlash, 0),
                      0};
    data[len - 1] = xorByteArray(data, len - 1);
    otaSendRequest(data, len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham vao che do ghi version moi cho STM32
    @ return        : true or false
*/
bool cmdWriteVersionStm32()
{
    // ghi du lieu vao bo nho STM32
    int len = 2;
    char data[len] = {CODE_FUNCTION_WRITE_VERSION >> 8,
                      CODE_FUNCTION_WRITE_VERSION & 0x00FF};
    otaSendRequest(data, len);
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham thuc hien ghi version moi cho STM32
    @const char *version      : Version moi can ghi cho STM32
    @return                   : true or false
*/
bool writeNewVersionStm32(const char *version)
{
    int len = 6;
    char data[len] = {version[0], version[1], version[2],
                      version[3], version[4], 0};
    data[len - 1] = xorByteArray(data, len - 1);
    otaSendRequest(data, 5);  // theem ack
    bool ret = otaCheckAck(TIME_CHECK_ACK);
    if (ret)
    {
        return true;
    }
    return false;
}

/*
    Ham thuat toan xorByte cho
    @const char *byteArray  : chuoi data can xor byte
    @int length             : do dai chuoi
*/
char xorByteArray(const char *byteArray, int length)
{
    // XOR byte đầu tiên với các byte tiếp theo
    char data = byteArray[0];
    for (int i = 1; i < length; i++)
    {
        data ^= byteArray[i];
    }
    return data;
}

/*
    Ham thuc hien tang dan dia chi ghi bo nho tren STM32
    @char *loadAddr : dia chi hien tai dua vafo thujc hien set dia chi moi
*/
void incrementLoadAddress(char *loadAddr)
{
    loadAddr[2] += 0x1;

    if (loadAddr[2] == 0)
    {
        loadAddr[1] += 0x1;

        if (loadAddr[1] == 0)
        {
            loadAddr[0] += 0x1;
        }
    }
}

/*
    Ham flash page with 256byte for STM32
    @ const char *address    : dia chi bat dau ghi
    @ const char *data       : data ghi
    @ return                 : true or false
*/
bool flashPage(const char *address, const char *data)
{
    // vafo che do ghi flash
    if (!cmdWriteFlashStm32())
    {
        return false;
    }
    // chon dia chi thanh ghi bat dau
    if (!setRegisterToAppFlash(address))
    {
        return false;
    }

    char crc = 0xFF;
    char size = 0xFF;
    uartWrite(&size, 1);
    for (int i = 0; i <= size; i++)
    {
        uartWrite(&data[i], 1);
        crc ^= data[i];
    }
    uartWrite(&crc, 1);
    bool ret = otaCheckAck(TIME_WRITE_FLASH_ACK);
    if (ret)
    {
        return true;
    }
    ESP_LOGE(__FUNCTION__, "Flash Page Erorr");
    return false;
}

/*
    Ham thuc hien doc data tu file .bin trong Spiffs va call hafm write cho
   STM32
   @ FILE *flash_file : ten file .bin firmware cua STm32
   @ return           : true or flase
*/
bool writeTaskSpiffs(FILE *flash_file)
{
    ESP_LOGI(__FUNCTION__, "Write Task");
    // config offset addres application stm32
    char loadAddress[4] = {START_ADDRESS_OTA_1, START_ADDRESS_OTA_2,
                           START_ADDRESS_OTA_3, START_ADDRESS_OTA_4};
    char block[CHUNK_SIZE_OTA] = {0};
    int currBlock = 0;
    int butesRead = 0;

    fseek(flash_file, 0, SEEK_SET);
    // setupSTM();

    while ((butesRead = fread(block, 1, CHUNK_SIZE_OTA, flash_file)) > 0)
    {
        int count = 3;
        currBlock++;
        ESP_LOGI(__FUNCTION__, "Writing block: %d", currBlock);

        if (!flashPage(loadAddress, block))
        {
            while (count != 0)
            {
                ESP_LOGW(__FUNCTION__, "Retry Write Page");
                if (flashPage(loadAddress, block))
                {
                    count = 3;
                    goto upAddress;
                }
                count--;
            }
            return false;
        }
    upAddress:
        incrementLoadAddress(loadAddress);
        memset(block, 0xff, CHUNK_SIZE_OTA);
    }
    return true;
}

/*
    Ham thuc hien doc file Bin trong file Spiffs thuc hien flash STM32
*/
void flashStm32viaSpiffs(const char *fileName, char *newVersion)
{
    // esp_err_t err = ESP_FAIL;
    ESP_LOGI(__FUNCTION__, "----------------Write Flash Stm32---------------");
    // max length path contain data
    char filePath[156];
    sprintf(filePath, "%s%s", PATH_SYSTEM, fileName);
    ESP_LOGI(__FUNCTION__, "File name: %s", filePath);

    // access Boot mode STm32
    if (!cmdStartBootStm32())
    {
        ESP_LOGE(__FUNCTION__, "Call Boot Erorr");
        // return;
    }
    ESP_LOGI(__FUNCTION__, "1.mode boot");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // todo read flash
    // Vào che do xoa bo nhơ và xoa bọ nho vung Appycation
    if (!cmdEraseFlashStm32())
    {
        ESP_LOGE(__FUNCTION__, "CMD Erase Erorr");
        if (!cmdExitBootStm32())
        {
            ESP_LOGE(__FUNCTION__, "Cannot exit boot");
        }
        return;
    }
    ESP_LOGI(__FUNCTION__, "2.vao mode xoa thanh cong");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // thuc hien xoa bo nho Applycation
    if (!eraseFlashMemoryAppStm32())
    {
        ESP_LOGE(__FUNCTION__, "Erase Application Success");
        if (!cmdExitBootStm32())
        {
            ESP_LOGE(__FUNCTION__, "Cannot exit boot");
        }
        return;
    }
    ESP_LOGI(__FUNCTION__, "3.xoa thanh cong");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    FILE *flash_file = fopen(filePath, "rb");
    if (flash_file != NULL)
    {
        do
        {
            ESP_LOGI(__FUNCTION__, "Writing STM32 Memory");
            if (!writeTaskSpiffs(flash_file))
            {
                ESP_LOGE(__FUNCTION__, "Write STM32 Memory False");
                break;
            }

            ESP_LOGI(__FUNCTION__, "STM32 Flashed Successfully!!!");
        } while (0);
    }

    // Write new version for stm32
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (!cmdWriteVersionStm32())
    {
        ESP_LOGE(__FUNCTION__, "Vao che do ghi version loi");
        fclose(flash_file);
        return;
    }
    if (!writeNewVersionStm32(newVersion))
    {
        ESP_LOGE(__FUNCTION__, "Ghi version loi");
        fclose(flash_file);
        return;
    }
    // exit boot mode
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(__FUNCTION__, "%s", "Ending Connection");
    if (!cmdExitBootStm32())
    {
        ESP_LOGE(__FUNCTION__, "Cannot exit boot");
    }
    ESP_LOGI(__FUNCTION__, "%s", "Closing file");
    fclose(flash_file);
}

/*
    Ham ghi bo nho STM32 voi kich thuoc tuy chinh
    @ char *address      : dia chi bat dau ghi
    @ const char *data   : data can ghi vao
    @ char *address      : kich thuoc van ghi luu y chia het cho 256
    @ retunr
*/
bool writeFlashStm32(char *address, const char *data, int sizeFlash)
{
    int numberPage = 0;
    int count = 3;
    int numberLoop = sizeFlash / CHUNK_SIZE_OTA;
    for (int i = 0; i < numberLoop; i++)
    {
        if (!flashPage(address, &data[i * CHUNK_SIZE_OTA]))
        {
            while (count != 0)
            {
                ESP_LOGW(__FUNCTION__, "Retry Write Page");
                if (flashPage(address, &data[i * CHUNK_SIZE_OTA]))
                {
                    count = 3;
                    goto upAddress;
                }
                count--;
            }
            return false;
        }
    upAddress:
        incrementLoadAddress(address);
    }
    return true;
}

/*
    Ham Get CRC32 cua STm32 sau khi OTA
    @ uint32_t *checksum : gia tri CRC tra ve tu STM32
    @ return             : true or false
*/
bool getCrcStm32Flash(uint32_t *checksum)
{
    int len = 2;
    char data[len] = {CODE_FUNCTION_GET_CRC32 >> 8,
                      CODE_FUNCTION_GET_CRC32 & 0x00FF};
    int lenReciver = 7;  // length data send STM32
    char dataReciver[lenReciver] = {0};

    otaSendRequest(data, len);

    if (uartRead(dataReciver, lenReciver, 1000) != 0)
    {
        if (dataReciver[0] != STM_ACK || dataReciver[6] != STM_ACK)
        {
            return false;
        }
        char xorCheck = xorByteArray(&dataReciver[1], 4);
        if (xorCheck != dataReciver[5])
        {
            return false;
        }
        *checksum = (uint32_t)dataReciver[4] << 24 |
                    (uint32_t)dataReciver[3] << 16 |
                    (uint32_t)dataReciver[2] << 8 | (uint32_t)dataReciver[1];
        return true;
    }

    return false;
}

/*
    send goi tin kiem tra trang thai cua STm32
*/
bool getStatusStm32(uint8_t *evenId)
{
    int len = 8;
    int crc = 0;
    uint8_t quantity = 1;
    uint8_t buff[len] = {0};
    buff[0] = MODBUS_ADDRESS_RCU_DEFAULT;
    buff[1] = FUNCTION_CODE_GET_VERSION;
    buff[2] = HI_UINT16(ADDRESS_CHECK_STATUS_STM32);
    buff[3] = LO_UINT16(ADDRESS_CHECK_STATUS_STM32);
    buff[4] = HI_UINT16(quantity);
    buff[5] = LO_UINT16(quantity);
    crc = crc16(buff, len - 2);
    buff[6] = LO_UINT16((uint16_t)crc);
    buff[7] = HI_UINT16((uint16_t)crc);
    if (xSemaphoreTake(xMutexModbusPort, (TickType_t)1000) == pdTRUE)
    {
        modbusWrite((char *)buff, len);
        int lenData = 7;
        char data[lenData] = {0};

        if (!modbusRead((char *)data))
        {
            ESP_LOGE(__FUNCTION__, "1.STM32 Inactive");
            xSemaphoreGive(xMutexModbusPort);
            return false;
        }

        // free key semafore
        xSemaphoreGive(xMutexModbusPort);
        crc = crc16((uint8_t *)data, lenData - 2);
        if (data[lenData - 1] != HI_UINT16(crc) ||
            data[lenData - 2] != LO_UINT16(crc))
        {
            ESP_LOGE(__FUNCTION__, "2.STM32 Inactive");
            return false;
        }
        uint16_t statusCode = (data[3] << 8) | data[4];
        switch (statusCode)
        {
            case STM32_OK:
            {
                *evenId = STM32_OK;
                return true;
            }
            // todo evenID erorr code
            default:
                return false;
        }
    }

    ESP_LOGE(__FUNCTION__, "Semaphore take error");
    return false;
}
