# Firmware Handover – Golden Imperial Project

**Author:** thanghd (thanghd@elifetech.vn)  
**Date:** 24/04/2025  
**Description:** Bàn giao firmware cho dự án **Golden Imperial**

---

## 1. Phạm vi bàn giao

### 1.1 STM32
- Source code: thư mục `STM32_GODDEN_IMPERIAL`
- Môi trường build: **Arduino 2.3.3**
- File output: **.bin cho STM32**

### 1.2 ESP32
- Source code: thư mục `ESP32_GODEN_IMPERIAL`
- Môi trường build: **ESP-IDF 5.3.3**
- File config:
  - Hướng dẫn setup
  - Build OTA cho hệ thống

---

## 2. Hướng dẫn nạp firmware cho ESP32

- Tool sử dụng: **ESP32 Flash Download Tool v3.9.3**
- Các file cần nạp:

| File | Address |
|-----|---------|
| `bootloader.bin` | `0x1000` |
| `partition_table.bin` | `0x8000` |
| `ota_data_initial.bin` | `0xD000` |
| `RCU_V2.bin` | `0x10000` |

- Chọn đúng cổng **COM**
- Thực hiện **Flash**
- Chờ quá trình nạp hoàn tất

---

## 3. Hướng dẫn cấu hình RCU

- **Bước 1:**  
  Nạp firmware cho **ESP32** và **STM32** thủ công

- **Bước 2:**  
  Cấp nguồn cho RCU  
  Kết nối Wi-Fi:
  - Tên: `RCU-XXXXXXXX`  
  - `XXXXXXXX` là **MAC address của RCU**  
  - Password: `11223344`

- **Bước 3:**  
  Truy cập địa chỉ:  `http://192.168.4.1/`
để vào giao diện cấu hình

- **Bước 4:**  
- Xóa file config cũ
- Upload file config mới với tên mặc định: file template on folder Document
  ```
  systemConfig.txt
  ```

- **Bước 5:**  
Reset RCU  
Theo dõi **trạng thái** và **version firmware** trên **ThingsBoard**

---

## 4. Hướng dẫn build firmware RCU

- **Bước 1:**  
Mở source code bằng **ESP-IDF**

- **Bước 2:**  
Vào file: `HttpsOtaRcu.c`

Cập nhật **version firmware**

- **Bước 3:**  
Upload firmware lên **ThingsBoard**  
Add firmware cho thiết bị tương ứng

- **Bước 4:**  
Theo dõi **trạng thái OTA** và **version firmware** trên **Dashboard**

---
