
#ifndef __UARTINIT_H__
#define __UARTINIT_H__

#include "sdkconfig.h"

#define UART_OTA_STM32_TXD (15)
#define UART_OTA_STM32_RXD (4)
#define UART_OTA_STM32_RTS (UART_PIN_NO_CHANGE)
#define UART_OTA_STM32_CTS (UART_PIN_NO_CHANGE)

#define UART_OTA_PORT_NUM (UART_NUM_1)
#define UART_OTA_BAUD_RATE (115200)
#define OTA_TASK_STACK_SIZE (1024 * 2)
#define BUF_SIZE (1024)

#ifdef __cplusplus
extern "C"
{
#endif
    void initUartOTA();
    void uartWrite(const char *data, int len);
    int uartRead(void *data, int size, int timeOut);
#ifdef __cplusplus
}
#endif

#endif /*__UARTINIT_H__*/
