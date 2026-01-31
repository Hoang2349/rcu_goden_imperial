#ifndef __MAPRULE_H__
#define __MAPRULE_H__

#include <iostream>

#include "Global.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void insertScenario(std::string key, rule_t value);
    void getScenario(std::string key, rule_t *output);
    void insertRegister(std::string key, uint16_t value);
    void getRegister(std::string key, uint16_t *output);

    void displayRe(rule_t data);
#ifdef __cplusplus
}
#endif

#endif