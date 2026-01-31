#ifndef __PARSESCEN_H__
#define __PARSESCEN_H__

#include <stdio.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void parseDataJsonScenLocal(const char *jsonString);

    uint8_t pushDataToMap(cJSON *item);

    void parseDataObject(cJSON *item, uint8_t *type, uint32_t *addr,
                         uint8_t *value);

    char *getKeyValuefromJson(cJSON *input);

    uint32_t strToU32(char *input);

    uint8_t findValue(char *str);

    uint8_t findType(char *str);

#ifdef __cplusplus
}
#endif

#endif

