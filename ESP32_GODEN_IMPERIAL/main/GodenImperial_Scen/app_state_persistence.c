#include "app_state_persistence.h"
#include "app_goden_imperial_common.h"
#include "app_control_output.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

#define TAG "STATE_PERSISTENCE"

// Static variable to hold the state in RAM
static persisted_state_t g_stored_state = {0};

/**
 * @brief Save the current state of all relays and input states to RAM
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t save_current_state_to_ram(void)
{
    // Copy current relay states
    g_stored_state.relay_states = status_room_cur;
    
    // Copy current sensor states
    g_stored_state.sensor_states = status_sensor;
    
    // Copy current outdoor states
    g_stored_state.outdoor_states = status_outdoor;
    
    // Copy current PMS states
    g_stored_state.pms_states = pms_room_status;
    
    // Set timestamp and validity flag
    g_stored_state.timestamp = (uint32_t)(esp_timer_get_time() / 1000000); // Convert to seconds
    g_stored_state.valid = 1;
    
    ESP_LOGI(TAG, "Successfully saved state to RAM");
    
    return ESP_OK;
}

/**
 * @brief Load the previously saved state from RAM
 * @param[out] loaded_state Pointer to structure where loaded state will be stored
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t load_state_from_ram(persisted_state_t *loaded_state)
{
    if (!loaded_state) {
        ESP_LOGE(TAG, "Invalid parameter: loaded_state is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Check if the stored state is valid
    if (g_stored_state.valid != 1) {
        ESP_LOGW(TAG, "Stored state is not valid");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Copy the stored state to the output parameter
    *loaded_state = g_stored_state;
    
    ESP_LOGI(TAG, "Successfully loaded state from RAM");
    return ESP_OK;
}

/**
 * @brief Restore the system state from RAM
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t restore_state_from_ram(void)
{
    esp_err_t ret;
    persisted_state_t loaded_state = {0};
    
    ret = load_state_from_ram(&loaded_state);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Could not load state from RAM, using default state: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Apply the loaded states to the current system
    status_room_cur = loaded_state.relay_states;
    status_sensor = loaded_state.sensor_states;
    status_outdoor = loaded_state.outdoor_states;
    pms_room_status = loaded_state.pms_states;
    
    ESP_LOGI(TAG, "Successfully restored system state from RAM");
    
    // Update the output states to match the restored state
    update_outputs_to_match_state();
    
    return ESP_OK;
}

/**
 * @brief Update all outputs to match the current state in memory
 * This function sends Modbus commands to ensure physical outputs match the stored state
 */
void update_outputs_to_match_state(void)
{
    // Update all relay states to match the stored state
    // This ensures that if the ESP32 restarted, the physical relays match the stored state

    // Master M1
    handle_event_master_m1(status_room_cur.master_m1_status);

    // Toilet
    handle_event_toilet(status_room_cur.toilet_status);

    // WC Light
    handle_event_wc_light(status_room_cur.wc_light_status);

    // Minibar
    handle_event_minibar(status_room_cur.minibar_status);

    // Master M2
    handle_event_master_m2(status_room_cur.master_m2_status);

    // Reading S2
    handle_event_reading_s2(status_room_cur.reading_s2_status);

    // Ceiling S2
    handle_event_ceiling_s2(status_room_cur.ceiling_light_s2_status);

    // Night Light S2
    handle_event_night_s2(status_room_cur.night_light_s2_status);

    // Night Light S3
    handle_event_night_s3(status_room_cur.night_light_s3_status);

    // Master M3
    handle_event_master_m3(status_room_cur.master_m3_status);

    // Ceiling S3
    handle_event_ceiling_s3(status_room_cur.ceiling_light_s3_status);

    // Reading S3
    handle_event_reading_s3(status_room_cur.reading_s3_status);

    // Update outdoor indicators
    scene_dnd(status_outdoor.status_dnd);
    scene_mur(status_outdoor.status_mur);

    ESP_LOGI(TAG, "Updated all outputs to match restored state");
}

/**
 * @brief Clear the saved state in RAM
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t clear_saved_state_from_ram(void)
{
    // Reset the stored state to zero
    memset(&g_stored_state, 0, sizeof(persisted_state_t));
    
    ESP_LOGI(TAG, "Successfully cleared saved state from RAM");
    
    return ESP_OK;
}

/**
 * @brief Initialize the state persistence system (RAM version)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t init_state_persistence(void)
{
    // Initialize the stored state to zero
    memset(&g_stored_state, 0, sizeof(persisted_state_t));
    
    ESP_LOGI(TAG, "RAM-based state persistence system initialized");
    return ESP_OK;
}

/**
 * @brief Get the current room status
 * @return Pointer to the current status_room_t structure
 */
status_room_t* get_current_room_status(void)
{
    return &status_room_cur;
}

/**
 * @brief Get the current sensor status
 * @return Pointer to the current status_sensor_t structure
 */
status_sensor_t* get_current_sensor_status(void)
{
    return &status_sensor;
}

/**
 * @brief Get the current outdoor status
 * @return Pointer to the current status_outdoor_t structure
 */
status_outdoor_t* get_current_outdoor_status(void)
{
    return &status_outdoor;
}

/**
 * @brief Get the current PMS status
 * @return Pointer to the current pms_room_status_t structure
 */
pms_room_status_t* get_current_pms_status(void)
{
    return &pms_room_status;
}