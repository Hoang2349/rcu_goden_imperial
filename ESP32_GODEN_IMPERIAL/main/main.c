#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "AppApConfig.h"
#include "AppEthernet.h"
#include "AppGpio.h"
#include "AppSpiffs.h"
#include "AppSystem.h"
#include "HttpsOtaRcu.h"
#include "MapRuleMqtt.h"
#include "Modbus.h"
#include "Mqtt.h"
#include "RuleEngine.h"
#include "otaStm32.h"

#include "app_main_goden.h"
void app_main(void)
{
    espSystemInit();
    appWifiApInit();
    appSpiffsInit();
    appEthernetInit();

    appSetupGpio();
    modbusInit();
    // init functuion for PR Goden Imperial
    appMqttInit();
    appHttpsOtaInit();
    init_goden_imperial();
    while (true)
    {
        logDebugHeap();
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}