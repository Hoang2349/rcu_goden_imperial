#ifndef APP_ADVANCED_STATE_MANAGEMENT_H
#define APP_ADVANCED_STATE_MANAGEMENT_H

#include "app_goden_imperial_common.h"
#include "app_state_persistence.h"  // Include the basic persistence structure
#include "esp_timer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lưu trạng thái hiện tại của phòng khi ở trạng thái OCC
 */
void save_occ_initial_state(void);

/**
 * @brief Khôi phục trạng thái đã lưu khi chuyển từ standby về OCC
 */
void restore_occ_initial_state(void);

/**
 * @brief Xóa trạng thái OCC đã lưu
 */
void clear_saved_occ_state(void);

/**
 * @brief Kiểm tra xem trạng thái OCC đã được lưu chưa
 * @return true nếu đã lưu, false nếu chưa
 */
bool is_occ_state_saved(void);

/**
 * @brief Lấy con trỏ đến trạng thái OCC đã lưu (cho mục đích debug)
 */
persisted_state_t* get_saved_occ_state(void);

#ifdef __cplusplus
}
#endif

#endif // APP_ADVANCED_STATE_MANAGEMENT_H