
#include "uartInit.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/// @brief init port uart for update OTA
void initUartOTA()
{
    uart_config_t uart_config = {
        .baud_rate = UART_OTA_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_OTA_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_OTA_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_OTA_PORT_NUM, UART_OTA_STM32_TXD, UART_OTA_STM32_RXD, UART_OTA_STM32_RTS, UART_OTA_STM32_CTS));
}

/// @brief Read data recive on uart port
/// @param data : point to data store
/// @param size : size of data
/// @param timeOut : time out check port
/// @return 0 - no data !=  have data
int uartRead( void *data, int size, int timeOut)
{
    return uart_read_bytes(UART_OTA_PORT_NUM, data, size, timeOut / portTICK_PERIOD_MS);
}


/// @brief Send data uarrt
/// @param data : data need send
/// @param len : length data send
void uartWrite(const char *data, int len)
{
    uart_write_bytes(UART_OTA_PORT_NUM, data, len);
}
