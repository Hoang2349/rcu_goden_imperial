# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS	main.c        						 # list the source files of this component
    		
			driver/EspSystem/AppSystem.c
			driver/Ethernet/AppEthernet.c
			driver/Ethernet/ethernet_init.c
    		driver/Spiffs/AppSpiffs.c
    		driver/OtaRcu/HttpsOtaRcu.c
			driver/GpioConfig/AppGpio.c
			driver/Modbus/Modbus.cpp

    		driver/Mqtt/Mqtt.c
			driver/Mqtt/MapRuleMqtt/MapRuleMqtt.cpp
			driver/Mqtt/MqttDeviceBirth.cpp
			driver/Mqtt/MqttDeviceData.c
			driver/Mqtt/MqttDeviceCommand.c


			driver/APConfig/AppApConfig.c
			driver/HttpServer/AppHttpServer.c
			RuleEngine/RuleEngine.cpp
			RuleEngine/MapRuleOffline/MapRule.cpp
			RuleEngine/ParseScenarioOffline/ParseScen.cpp
			RuleEngine/QueueInput/QueueInput.cpp
			RuleEngine/QueueModbusOutput/QueueModbusOutput.cpp
			RuleEngine/HandleOutputQueue/HandleOutputQueue.cpp

			driver/Sparkplug/pb_common.c
			driver/Sparkplug/pb_decode.c
			driver/Sparkplug/pb_encode.c
			driver/Sparkplug/tahu.c
			driver/Sparkplug/tahu_pb.c

			ModuleOTAStm32/otaStm32.cpp
    		ModuleOTAStm32/configHarware/uartInit.cpp

			GodenImperial_Scen/app_main_goden.c
			GodenImperial_Scen/app_control_output.c
			GodenImperial_Scen/app_handle_input.c

    INCLUDE_DIRS  	driver 
					driver/Sparkplug    					 # optional, add here public include directories
    				driver/Ethernet
    				driver/Spiffs
    				driver/OtaRcu
					driver/APConfig
					driver/HttpServer
					driver/GpioConfig
					driver/Modbus
    				driver/Mqtt
					driver/Mqtt/MapRuleMqtt
					driver/EspSystem
					
				  	RuleEngine
					RuleEngine/MapRuleOffline
					RuleEngine/ParseScenarioOffline
					RuleEngine/QueueInput
					RuleEngine/QueueModbusOutput
					RuleEngine/HandleOutputQueue

					ModuleOTAStm32
    				ModuleOTAStm32/configHarware

					GodenImperial_Scen
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)




