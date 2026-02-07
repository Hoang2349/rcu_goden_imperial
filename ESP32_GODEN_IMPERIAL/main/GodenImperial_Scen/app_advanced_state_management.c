#include "app_advanced_state_management.h"
#include "app_goden_imperial_common.h"
#include "app_control_output.h"
#include "esp_log.h"
#include <string.h>

#define TAG "ADVANCED_STATE_MGT"

// Cấu trúc lưu trữ trạng thái OCC ban đầu
static persisted_state_t g_occ_initial_state = {0};
static bool g_occ_state_saved = false;

/**
 * @brief Lưu trạng thái hiện tại của phòng khi ở trạng thái OCC
 */
void save_occ_initial_state(void)
{
    // Sao chép trạng thái hiện tại vào bộ nhớ tạm
    g_occ_initial_state.relay_states = status_room_cur;
    g_occ_initial_state.sensor_states = status_sensor;
    g_occ_initial_state.outdoor_states = status_outdoor;
    g_occ_initial_state.pms_states = pms_room_status;
    g_occ_initial_state.timestamp = (uint32_t)(esp_timer_get_time() / 1000000); // Convert to seconds
    g_occ_initial_state.valid = 1;
    
    g_occ_state_saved = true;
    
    ESP_LOGI(TAG, "Saved initial OCC state to memory");
}

/**
 * @brief Khôi phục trạng thái đã lưu khi chuyển từ standby về OCC
 */
void restore_occ_initial_state(void)
{
    if (!g_occ_state_saved || g_occ_initial_state.valid != 1) {
        ESP_LOGW(TAG, "No valid OCC state to restore");
        return;
    }
    
    // Khôi phục trạng thái relay
    status_room_cur = g_occ_initial_state.relay_states;
    
    // Khôi phục trạng thái cảm biến
    status_sensor = g_occ_initial_state.sensor_states;
    
    // Khôi phục trạng thái outdoor
    status_outdoor = g_occ_initial_state.outdoor_states;
    
    // Khôi phục trạng thái PMS
    pms_room_status = g_occ_initial_state.pms_states;
    
    ESP_LOGI(TAG, "Restored initial OCC state from memory");
    
    // Cập nhật các output vật lý để khớp với trạng thái đã khôi phục
    update_outputs_to_match_state();
}

/**
 * @brief Xóa trạng thái OCC đã lưu
 */
void clear_saved_occ_state(void)
{
    memset(&g_occ_initial_state, 0, sizeof(persisted_state_t));
    g_occ_state_saved = false;
    
    ESP_LOGI(TAG, "Cleared saved OCC state");
}

/**
 * @brief Kiểm tra xem trạng thái OCC đã được lưu chưa
 * @return true nếu đã lưu, false nếu chưa
 */
bool is_occ_state_saved(void)
{
    return g_occ_state_saved;
}

/**
 * @brief Lấy con trỏ đến trạng thái OCC đã lưu (cho mục đích debug)
 */
persisted_state_t* get_saved_occ_state(void)
{
    if (g_occ_state_saved) {
        return &g_occ_initial_state;
    }
    return NULL;
}