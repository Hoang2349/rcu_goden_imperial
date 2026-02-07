#ifndef APP_STATE_PERSISTENCE_H
#define APP_STATE_PERSISTENCE_H

#include "esp_err.h"
#include "app_goden_imperial_common.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the state persistence system
 * This function initializes NVS and attempts to restore the previous state
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t init_state_persistence(void);

/**
 * @brief Save the current state of all relays and input states to RAM
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t save_current_state_to_ram(void);

/**
 * @brief Restore the system state from RAM
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t restore_state_from_ram(void);

/**
 * @brief Update all outputs to match the current state in memory
 * This function sends Modbus commands to ensure physical outputs match the stored state
 */
void update_outputs_to_match_state(void);

/**
 * @brief Clear the saved state in NVS
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t clear_saved_state_from_nvs(void);

#ifdef __cplusplus
}
#endif

#endif // APP_STATE_PERSISTENCE_H