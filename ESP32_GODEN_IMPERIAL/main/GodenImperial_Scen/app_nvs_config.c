#include "nvs_flash.h"
#include "app_nvs_config.h"
#include <esp_err.h>
#include <esp_log.h>

#include <stdint.h>
#include <stddef.h>

const static char *TAG = "NVS_ESP32";

#define NVS_NAME "goden_imperial_nvs"

// === NVS ESP32 IMPLEMENTATION === //

int goden_imperial_nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "NVS init failed, erasing NVS partition and retrying...");
        ret = nvs_flash_erase();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to erase NVS flash, error code: %d", ret);
            return ret;
        }
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize NVS flash, error code: %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "NVS initialized successfully");
    return ESP_OK;
}

int goden_imperial_nvs_read(const char *key, void *out_value, size_t *length)
{
    if (!key || !out_value || !length)
    {
        ESP_LOGE(TAG, "Invalid argument for NVS read");
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAME, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS handle: %d", ret);
        return ret;
    }
    ret = nvs_get_blob(nvs_handle, key, out_value, length);
    if (ret == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        nvs_close(nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    else if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error reading from NVS: %d", ret);
        nvs_close(nvs_handle);
        return ret;
    }
    nvs_close(nvs_handle);
    return ESP_OK;
}

int goden_imperial_nvs_write(const char *key, const void *value, size_t length)
{
    if (!key || !value || length == 0)
    {
        ESP_LOGE(TAG, "Invalid argument for NVS write");
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAME, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS handle: %d", ret);
        return ret;
    }
    ret = nvs_set_blob(nvs_handle, key, value, length);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error writing to NVS: %d", ret);
        nvs_close(nvs_handle);
        return ret;
    }
    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error committing NVS changes: %d", ret);
        nvs_close(nvs_handle);
        return ret;
    }
    nvs_close(nvs_handle);
    return ESP_OK;
}

int goden_imperial_nvs_erase(const char *key)
{
    if (!key)
        return ESP_ERR_INVALID_ARG;
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAME, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
        return ret;
    ret = nvs_erase_key(nvs_handle, key);
    if (ret != ESP_OK)
    {
        nvs_close(nvs_handle);
        return ret;
    }
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret;
}

int goden_imperial_nvs_deinit()
{
    ESP_LOGI(TAG, "Clearing NVS flash...");
    int ret = nvs_flash_erase();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to erase NVS flash: %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "NVS flash cleared successfully");
    return ESP_OK;
}

// === END NVS ESP32 IMPLEMENTATION === //
int goden_inperial_read_config_data(app_nvs_config_t *config_data)
{
    if (!config_data)
    {
        return ESP_ERR_INVALID_ARG;
    }
    size_t length = sizeof(app_nvs_config_t);
    return goden_imperial_nvs_read(KEY_CONFIG_DATA, config_data, &length);
}

int goden_inperial_write_config_data(const app_nvs_config_t *config_data)
{
    if (!config_data)
    {
        return ESP_ERR_INVALID_ARG;
    }
    return goden_imperial_nvs_write(KEY_CONFIG_DATA, config_data, sizeof(app_nvs_config_t));
}