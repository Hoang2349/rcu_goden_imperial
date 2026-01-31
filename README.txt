* Author: thanghd(thanghd@elifetech.vn)
* Date: 24.4.2025
* Description: bàn giao firmware cho dự án Goden Imperial

Bao gồm: 
	- STM32:
		+ Source code thư mục STM32_GODDEN_IMPERIAL
		+ Môi trường build: Arduino 2.3.3
		+ File .bin cho STM32
	- ESP32: 
		+ Source code thư mục ESP32_GODEN_IMPERIAL
		+ Môi trường build: espidf 5.3.3
		+ File config cho hệ thống hướng dẫn Setup, Build OTA
* Hướng dẫn nạp code cho ESP32
	- Sử dụng tool: ESP32 FLASH DOWNLOAD TOOL V3.9.3
	- Đưa các file: 
		+ bootloader.bin 		-> 0x1000
		+ partition_table.bin 	-> 0x8000
		+ ota_data_initial.bin 	-> 0xD000
		+ RCU_V2.bin 			-> 0x10000
	- Lựa chọn cổng COM và thực hiện Flash 
	- Chờ cho kết thúc

* Hướng dẫn config cho RCU:
	- B1: Nạp firmware cho cả ESP32 và STM32 thủ công bằng tay
	- B2: cắm nguồn cho RCU, dùng điện thoại bắt wifi với tên
	RCU-XXXXXXXX với "XXXXXXXX" là đại chỉ MAC của RCU pass:11223344
	- B3: Truy cập vào địa chỉ 192.168.4.1/ vào giao diện config
	- B4: Xóa file config cũ và tải file config mới lên với tên file mặc định
	"systemConfig.txt" 
	- B5: Reset RCU và theo dõi trạng thái và version trên ThingsBoard.
	
	
* Hướng dẫn BUILD version firmware cho RCU
	- B1: mở source code build trên idf
	- B2: Vào file "HttpsOtaRcu.c" update version cho RCU
	- B3: Tải firmware lên ThingsBoard và add firmware cho thiết bị 
	- B4: Theo dõi trạng thái Update và Version thiết bị trên Dashboard