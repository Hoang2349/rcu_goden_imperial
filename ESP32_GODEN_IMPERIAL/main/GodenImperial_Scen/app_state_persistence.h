#ifndef APP_STATE_PERSISTENCE_H
#define APP_STATE_PERSISTENCE_H

#include "esp_err.h"
#include "app_goden_imperial_common.h"
#include <stdbool.h>

// Define the structure to store all relay states in RAM
typedef struct {
    status_room_t relay_states;
    status_sensor_t sensor_states;
    status_outdoor_t outdoor_states;
    pms_room_status_t pms_states;
    uint32_t timestamp;  // Unix timestamp when state was saved
    uint8_t valid;       // Flag to indicate if the stored data is valid
} persisted_state_t;

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
 * @brief Get the current room status
 * @return Pointer to the current status_room_t structure
 */
status_room_t* get_current_room_status(void);

/**
 * @brief Get the current sensor status
 * @return Pointer to the current status_sensor_t structure
 */
status_sensor_t* get_current_sensor_status(void);

/**
 * @brief Get the current outdoor status
 * @return Pointer to the current status_outdoor_t structure
 */
status_outdoor_t* get_current_outdoor_status(void);

/**
 * @brief Get the current PMS status
 * @return Pointer to the current pms_room_status_t structure
 */
pms_room_status_t* get_current_pms_status(void);

/**
 * @brief Clear the saved state in NVS
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t clear_saved_state_from_nvs(void);

#ifdef __cplusplus
}
#endif

#endif // APP_STATE_PERSISTENCE_H