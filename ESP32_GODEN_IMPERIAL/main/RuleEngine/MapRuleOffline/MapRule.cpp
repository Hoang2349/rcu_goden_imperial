#include "MapRule.h"

#include <stdlib.h>

#include <map>
#include <vector>

std::map<std::string, rule_t> scenario;
std::map<std::string, uint16_t> registermb;

/**
 * @brief ham them cap key value vào trong Map
 * @param1 string key kem can them vao
 * @param2 rule_t value kich ban in out cua key
 * @return
 */
void insertScenario(std::string key, rule_t value) 
{ scenario[key] = {value}; }

/**
 * @brief lay kịch bản ứng voi key
 * @param1 string key key can tim kiem
 * @param2 rule_t *output gia tri càn tim lưu vào biên con tro rule_t
 * @return
 */
void getScenario(std::string key, rule_t *output)
{
    auto data = scenario.find(key);
    if (data != scenario.end())
    {
        *output = data->second;
    }
}

/**
 * @brief ham them cap key value vào trong Map Lưu trang thai thiest bị
 * @param1 string key kem can them vao
 * @param2 uint16_t value kich ban in out cua key
 * @return
 */
void insertRegister(std::string key, uint16_t value)
{
    registermb[key] = {value};
}

/**
 * @brief lay thogn tin thanh ghi ung voi key
 * @param1 string key kem can them vao
 * @param2 rule_t value kich ban in out cua key
 * @return
 */
void getRegister(std::string key, uint16_t *output)
{
    auto data = registermb.find(key);
    if (data != registermb.end())
    {
        *output = data->second;
    }
}

/**
 * @brief debug thong tin kich ban
 * @param1 rule_t data bien can debug
 * @return
 */
void displayRe(rule_t data)
{
    printf("---------------------------\n");
    printf("---data  input---\n");
    printf("Type: 0x%X\n", data.input.type);
    printf("Address: 0x%lX\n", data.input.address);
    printf("value: 0x%X\n", data.input.value);
    printf("===data  output===\n");
    printf("Size output: %d\n", data.output.size());
    for (auto it = data.output.begin(); it != data.output.end(); ++it)
    {
        printf("Type: 0x%X\n", it->type);
        printf("Address: 0x%lX\n", it->address);
        printf("value: 0x%X\n", it->value);
    }
    printf("---------------------------\n");
}