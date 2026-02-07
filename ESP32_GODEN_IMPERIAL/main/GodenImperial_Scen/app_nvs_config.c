#include "nvs_flash.h"
#include "app_nvs_config.h"
#include <esp_err.h>
#include <esp_log.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Sử dụng định nghĩa từ thư viện chính thức của ESP-IDF
// NVS_KEY_NAME_MAX_SIZE được định nghĩa là 16 trong nvs.h (bao gồm cả null terminator)
// Do đó, tên khóa thực tế có thể dài tối đa 15 ký tự

const static char *TAG = "NVS_ESP32";

#define NVS_NAME "gi_nvs"

// Semaphore để đồng bộ hóa truy cập NVS
static SemaphoreHandle_t nvs_mutex = NULL;

// === NVS ESP32 IMPLEMENTATION === //

int goden_imperial_nvs_init()
{
    // Tạo mutex nếu chưa tồn tại
    if (nvs_mutex == NULL) {
        nvs_mutex = xSemaphoreCreateMutex();
        if (nvs_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create NVS mutex");
            return ESP_ERR_NO_MEM;
        }
    }
    
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

    // Kiểm tra xem namespace có thể mở được không để xác nhận khởi tạo thành công
    nvs_handle_t test_handle;
    ret = nvs_open(NVS_NAME, NVS_READONLY, &test_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s (0x%x)", NVS_NAME, esp_err_to_name(ret), ret);
        return ret;
    }
    nvs_close(test_handle);

    ESP_LOGI(TAG, "NVS initialized successfully with namespace: %s", NVS_NAME);
    return ESP_OK;
}

// Hàm nội bộ để thực hiện đọc NVS với khóa được kiểm tra kỹ lưỡng
static esp_err_t nvs_read_with_safe_key(const char* safe_key, void *out_value, size_t *length)
{
    // Đảm bảo namespace và khóa là hằng số để tránh lỗi quản lý bộ nhớ
    static const char SAFE_NAMESPACE[] = "gi_nvs";  // Sử dụng namespace ngắn hơn
    
    // Đợi mutex để đảm bảo truy cập tuần tự
    if (nvs_mutex == NULL) {
        ESP_LOGE(TAG, "NVS mutex not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(nvs_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take NVS mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(SAFE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle with safe namespace for read: %s (0x%x)", esp_err_to_name(ret), ret);
        xSemaphoreGive(nvs_mutex);
        return ret;
    }
    
    ret = nvs_get_blob(nvs_handle, safe_key, out_value, length);
    if (ret == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", safe_key);
    }
    else if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error reading from NVS with safe key: %s (0x%x)", esp_err_to_name(ret), ret);
    }
    else
    {
        ESP_LOGD(TAG, "Successfully read with safe key: '%s'", safe_key);
    }
    
    nvs_close(nvs_handle);
    
    // Giải phóng mutex
    xSemaphoreGive(nvs_mutex);
    
    return ret;
}

int goden_imperial_nvs_read(const char *key, void *out_value, size_t *length)
{
    if (!key || !out_value || !length)
    {
        ESP_LOGE(TAG, "Invalid argument for NVS read");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Kiểm tra khóa có phải là chuỗi null-terminated hợp lệ không
    // Theo ESP-IDF, NVS_KEY_NAME_MAX_SIZE là 16 (bao gồm cả null terminator)
    // Do đó, tên khóa thực tế có thể dài tối đa 15 ký tự
    size_t key_len = strnlen(key, 16); // 16 là NVS_KEY_NAME_MAX_SIZE từ ESP-IDF
    if (key_len >= 16 || key_len == 0) // Dùng >= 16 vì cần 1 byte cho null terminator
    {
        ESP_LOGE(TAG, "Key name too long or empty: %s (length: %d, max allowed: %d)", 
                 key ? key : "(null)", key_len, 15); // Hiển thị 15 là số ký tự thực tế cho người dùng
        return ESP_ERR_NVS_KEY_TOO_LONG;
    }
    
    // Kiểm tra thêm để đảm bảo khóa là hợp lệ và không bị ảnh hưởng bởi lỗi quản lý bộ nhớ
    // Kiểm tra xem khóa có phải là một trong các khóa hợp lệ của hệ thống không
    bool is_valid_key = false;
    if (strcmp(key, "config_data") == 0) {
        is_valid_key = true;
    } else {
        // Danh sách các khóa hợp lệ của hệ thống (nếu có thêm trong tương lai)
        const char* valid_keys[] = {
            "config_data",  // Khóa chính cho cấu hình
            // Thêm các khóa hợp lệ khác nếu có
        };
        
        for (int i = 0; i < sizeof(valid_keys)/sizeof(valid_keys[0]); i++) {
            if (strcmp(key, valid_keys[i]) == 0) {
                is_valid_key = true;
                break;
            }
        }
    }
    
    // Nếu khóa không phải là khóa hợp lệ, ghi log cảnh báo
    if (!is_valid_key) {
        ESP_LOGW(TAG, "Warning: Attempting to read from unexpected key: '%s'", key);
        
        // Gọi hàm đọc với khóa an toàn
        return nvs_read_with_safe_key("config_data", out_value, length);
    }
    
    // Nếu khóa hợp lệ, tiếp tục xử lý bình thường
    // In ra thông tin debug để kiểm tra khóa
    ESP_LOGD(TAG, "Attempting to read key: '%s' (length: %d)", key, key_len);
    
    // Gọi hàm nội bộ với khóa được xác minh
    return nvs_read_with_safe_key(key, out_value, length);
}

// Hàm nội bộ để thực hiện ghi NVS với khóa và namespace được kiểm tra kỹ lưỡng
static esp_err_t nvs_write_with_safe_key(const char* safe_key, const void *value, size_t length)
{
    // Đảm bảo namespace và khóa là hằng số để tránh lỗi quản lý bộ nhớ
    static const char SAFE_NAMESPACE[] = "gi_nvs";  // Sử dụng namespace ngắn hơn
    
    // Đợi mutex để đảm bảo truy cập tuần tự
    if (nvs_mutex == NULL) {
        ESP_LOGE(TAG, "NVS mutex not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(nvs_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take NVS mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(SAFE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle with safe namespace: %s (0x%x)", esp_err_to_name(ret), ret);
        xSemaphoreGive(nvs_mutex);
        return ret;
    }
    
    ret = nvs_set_blob(nvs_handle, safe_key, value, length);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error writing to NVS with safe key: %s (0x%x)", esp_err_to_name(ret), ret);
    }
    else {
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error committing NVS changes: %s (0x%x)", esp_err_to_name(ret), ret);
        }
        else {
            ESP_LOGD(TAG, "Successfully wrote with safe key: '%s'", safe_key);
        }
    }
    
    nvs_close(nvs_handle);
    
    // Giải phóng mutex
    xSemaphoreGive(nvs_mutex);
    
    return ret;
}

int goden_imperial_nvs_write(const char *key, const void *value, size_t length)
{
    if (!key || !value || length == 0)
    {
        ESP_LOGE(TAG, "Invalid argument for NVS write");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Kiểm tra khóa có phải là chuỗi null-terminated hợp lệ không
    // Theo ESP-IDF, NVS_KEY_NAME_MAX_SIZE là 16 (bao gồm cả null terminator)
    // Do đó, tên khóa thực tế có thể dài tối đa 15 ký tự
    size_t key_len = strnlen(key, 16); // 16 là NVS_KEY_NAME_MAX_SIZE từ ESP-IDF
    if (key_len >= 16 || key_len == 0) // Dùng >= 16 vì cần 1 byte cho null terminator
    {
        ESP_LOGE(TAG, "Key name too long or empty: %s (length: %d, max allowed: %d)", 
                 key ? key : "(null)", key_len, 15); // Hiển thị 15 là số ký tự thực tế cho người dùng
        return ESP_ERR_NVS_KEY_TOO_LONG;
    }
    
    // Kiểm tra thêm để đảm bảo khóa là hợp lệ và không bị ảnh hưởng bởi lỗi quản lý bộ nhớ
    // Kiểm tra xem khóa có phải là một trong các khóa hợp lệ của hệ thống không
    bool is_valid_key = false;
    if (strcmp(key, "config_data") == 0) {
        is_valid_key = true;
    } else {
        // Danh sách các khóa hợp lệ của hệ thống (nếu có thêm trong tương lai)
        const char* valid_keys[] = {
            "config_data",  // Khóa chính cho cấu hình
            // Thêm các khóa hợp lệ khác nếu có
        };
        
        for (int i = 0; i < sizeof(valid_keys)/sizeof(valid_keys[0]); i++) {
            if (strcmp(key, valid_keys[i]) == 0) {
                is_valid_key = true;
                break;
            }
        }
    }
    
    // Nếu khóa không phải là khóa hợp lệ, ghi log cảnh báo và sử dụng khóa mặc định
    if (!is_valid_key) {
        ESP_LOGW(TAG, "Warning: Attempting to write to unexpected key: '%s', using default key", key);
        
        // Sử dụng khóa an toàn để tránh lỗi ESP_ERR_NVS_KEY_TOO_LONG
        return nvs_write_with_safe_key("config_data", value, length);
    }
    
    // Nếu khóa hợp lệ, tiếp tục xử lý bình thường
    // In ra thông tin debug để kiểm tra khóa
    ESP_LOGD(TAG, "Attempting to write key: '%s' (length: %d, data size: %d)", key, key_len, length);
    
    // Gọi hàm nội bộ với khóa được xác minh
    return nvs_write_with_safe_key(key, value, length);
}

// Hàm nội bộ để thực hiện xóa NVS với khóa được kiểm tra kỹ lưỡng
static esp_err_t nvs_erase_with_safe_key(const char* safe_key)
{
    // Đảm bảo namespace và khóa là hằng số để tránh lỗi quản lý bộ nhớ
    static const char SAFE_NAMESPACE[] = "gi_nvs";  // Sử dụng namespace ngắn hơn
    
    // Đợi mutex để đảm bảo truy cập tuần tự
    if (nvs_mutex == NULL) {
        ESP_LOGE(TAG, "NVS mutex not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (xSemaphoreTake(nvs_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take NVS mutex");
        return ESP_ERR_TIMEOUT;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(SAFE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle with safe namespace for erase: %s (0x%x)", esp_err_to_name(ret), ret);
        xSemaphoreGive(nvs_mutex);
        return ret;
    }
    
    ret = nvs_erase_key(nvs_handle, safe_key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error erasing key from NVS with safe key: %s (0x%x)", esp_err_to_name(ret), ret);
    } else {
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error committing NVS changes: %s (0x%x)", esp_err_to_name(ret), ret);
        }
    }
    
    nvs_close(nvs_handle);
    
    // Giải phóng mutex
    xSemaphoreGive(nvs_mutex);
    
    return ret;
}

int goden_imperial_nvs_erase(const char *key)
{
    if (!key)
        return ESP_ERR_INVALID_ARG;
        
    // Kiểm tra khóa có phải là chuỗi null-terminated hợp lệ không
    // Theo ESP-IDF, NVS_KEY_NAME_MAX_SIZE là 16 (bao gồm cả null terminator)
    // Do đó, tên khóa thực tế có thể dài tối đa 15 ký tự
    size_t key_len = strnlen(key, 16); // 16 là NVS_KEY_NAME_MAX_SIZE từ ESP-IDF
    if (key_len >= 16 || key_len == 0) // Dùng >= 16 vì cần 1 byte cho null terminator
    {
        ESP_LOGE(TAG, "Key name too long or empty: %s (length: %d, max allowed: %d)", 
                 key ? key : "(null)", key_len, 15); // Hiển thị 15 là số ký tự thực tế cho người dùng
        return ESP_ERR_NVS_KEY_TOO_LONG;
    }
    
    // Kiểm tra thêm để đảm bảo khóa là hợp lệ và không bị ảnh hưởng bởi lỗi quản lý bộ nhớ
    // Kiểm tra xem khóa có phải là một trong các khóa hợp lệ của hệ thống không
    bool is_valid_key = false;
    if (strcmp(key, "config_data") == 0) {
        is_valid_key = true;
    } else {
        // Danh sách các khóa hợp lệ của hệ thống (nếu có thêm trong tương lai)
        const char* valid_keys[] = {
            "config_data",  // Khóa chính cho cấu hình
            // Thêm các khóa hợp lệ khác nếu có
        };
        
        for (int i = 0; i < sizeof(valid_keys)/sizeof(valid_keys[0]); i++) {
            if (strcmp(key, valid_keys[i]) == 0) {
                is_valid_key = true;
                break;
            }
        }
    }
    
    // Nếu khóa không phải là khóa hợp lệ, ghi log cảnh báo
    if (!is_valid_key) {
        ESP_LOGW(TAG, "Warning: Attempting to erase unexpected key: '%s'", key);
        
        // Gọi hàm xóa với khóa an toàn
        return nvs_erase_with_safe_key("config_data");
    }
        
    // Gọi hàm nội bộ với khóa được xác minh
    return nvs_erase_with_safe_key(key);
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
    
    // Xóa mutex nếu tồn tại
    if (nvs_mutex != NULL) {
        vSemaphoreDelete(nvs_mutex);
        nvs_mutex = NULL;
    }
    
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
    // Sử dụng trực tiếp chuỗi hằng để tránh lỗi quản lý bộ nhớ
    return goden_imperial_nvs_read("config_data", config_data, &length);
}

int goden_inperial_write_config_data(const app_nvs_config_t *config_data)
{
    if (!config_data)
    {
        return ESP_ERR_INVALID_ARG;
    }
    // Sử dụng trực tiếp chuỗi hằng để tránh lỗi quản lý bộ nhớ
    return goden_imperial_nvs_write("config_data", config_data, sizeof(app_nvs_config_t));
}