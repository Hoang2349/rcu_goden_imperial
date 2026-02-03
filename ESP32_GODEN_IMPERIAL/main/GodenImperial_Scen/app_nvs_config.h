#pragma once

#include <stdint.h>
#include <stddef.h>

#define KEY_CONFIG_DATA "config_data"

typedef struct app_nvs_config
{
    uint32_t time_crossing;
    double time_door_ajar;
} app_nvs_config_t;

#ifdef __cplusplus
extern "C" {
#endif
    int goden_imperial_nvs_init();
    int goden_imperial_nvs_deinit();
    int goden_imperial_nvs_erase(const char *key);

    int goden_inperial_read_config_data(app_nvs_config_t *config_data);
    int goden_inperial_write_config_data(const app_nvs_config_t *config_data);

#ifdef __cplusplus
}
#endif  