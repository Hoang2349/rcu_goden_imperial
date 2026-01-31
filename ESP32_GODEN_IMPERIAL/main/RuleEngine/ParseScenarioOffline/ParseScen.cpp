#include "ParseScen.h"

#include <string.h>

#include "Global.h"
#include "MapRule.h"
#include "MapRuleMqtt.h"
#include "cJSON.h"

/**
 * @brief hàm xử lý array Json kịch bản
 * @param1 *jsonString chuỗi kịch bản chạy Rule Engine
 * @return kêt quả hàm RULE_OK or RULE_ERROR
 */
void parseDataJsonScenLocal(const char *jsonString)
{
    cJSON *root = cJSON_Parse(jsonString);
    // printf("%s\n", jsonString);
    if (root == NULL)
    {
        return;
    }
    ESP_LOGI(__FUNCTION__, "Parse Json Success!!!");
    pushDataToMap(root);
    cJSON_Delete(root);
}

/**
 * @brief ham xu li JSOn dua du lieu scnario vao map
 * @param1 cJSON *item tuowng usng 1 kich ban
 * @return
 */
uint8_t pushDataToMap(cJSON *item)
{
    rule_t scen;
    rule_t scen1;
    std::vector<outputItem_t> temp = {};
    outputItem_t val = {};
    cJSON *input = cJSON_GetObjectItem(item, "input");
    cJSON *output = cJSON_GetObjectItem(item, "output");
    char *keyScen = getKeyValuefromJson(input);
    // Parse data input
    parseDataObject(input, &scen.input.type, &scen.input.address,
                    &scen.input.value);
    if (!cJSON_IsArray(output))
    {
        return RULE_ERROR;
    }
    int outputSize = cJSON_GetArraySize(output);
    for (int j = 0; j < outputSize; j++)
    {
        cJSON *outputItem = cJSON_GetArrayItem(output, j);
        parseDataObject(outputItem, &val.type, &val.address, &val.value);
        temp.push_back(val);
        // Insert key value to Table Register
        uint32_t b = (val.address >> 8) & 0xFFFFFF;
        char keyReg[7];
        snprintf(keyReg, sizeof(keyReg), "%06lX", b);
        insertRegister(keyReg, 0);
    }
    scen.output = temp;
    insertScenario(keyScen, scen);
    return RULE_OK;
}

/**
 * @brief hàm xử lý lấy keyvalue lưu hashtable
 * @param1 *input json chưa thông tin key
 * @return trả về key với *input đầu vào
 */
char *getKeyValuefromJson(cJSON *input)
{
    char *key = cJSON_GetObjectItem(input, "address")->valuestring;
    ESP_LOGI(__FUNCTION__, " %s", key);
    return key;
}

/**
 * @brief Hàm xử lí lấy các dữ liệu trong object output để đưa vào table
 * scenario đồng thồi đưa các giá trị key value mới vào hash table register
 * @param1 *tem là chuỗi array chứa thông tin 1 object output kịch bản
 * @param2 *type là type output
 * @param3 *address chưa địa chỉ thoogn tin tải điều khiển
 * @param4 * value chứa thông tin kiểu điều khiển
 * @return
 */
void parseDataObject(cJSON *item, uint8_t *type, uint32_t *address,
                     uint8_t *value)
{
    char *typ = cJSON_GetObjectItem(item, "type")->valuestring;
    char *addr = cJSON_GetObjectItem(item, "address")->valuestring;
    char *val = cJSON_GetObjectItem(item, "value")->valuestring;
    *type = findType(typ);
    *address = strToU32(addr);
    *value = findValue(val);
}

/**
 * @brief so sánh và tìm ra type của object
 * @param1 *str chuỗi cần so sánh
 * @return trả về type của object
 */
uint8_t findType(char *str)
{
    if (strcmp(str, "relay") == 0)
    {
        return relay;
    }
    if (strcmp(str, "outputRj45") == 0)
    {
        return outputRj45;
    }
    if (strcmp(str, "ledModbus") == 0)
    {
        return ledModbus;
    }
    return 99;
}

/**
 * @brief so sánh và tìm ra value điều khiển
 * @param1 *str chuỗi cần so sánh
 * @return trả về kiểu value cần điều khiển
 */
uint8_t findValue(char *str)
{
    // state control toggle value current
    if (strcmp(str, "toggle") == 0)
    {
        return RULE_TOGGLE;
    }
    // "1" values need set 1(ON)
    if (strcmp(str, "1") == 0)
    {
        return RULE_ON;
    }
    // "0" values need set 0(OFF)
    if (strcmp(str, "0") == 0)
    {
        return RULE_OFF;
    }
    // no handle
    return 99;
}

/**
 * @brief hàm xử lí đưa address từ string to uint32_t
 * @param1 *input chuỗi cần chuổi đổi
 * @return uint16_t giá trí sau chuyển đổi
 */
uint32_t strToU32(char *input)
{
    char *endPtr;
    int intValue = (int)strtol(input, &endPtr, 16);
    if (*endPtr != '\0')
    {
        return RULE_ERROR;
    }
    ESP_LOGI(__FUNCTION__, "Hex addr : 0x%X", intValue);
    return intValue;
}