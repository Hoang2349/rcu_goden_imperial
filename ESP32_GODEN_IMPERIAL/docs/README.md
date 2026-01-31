/* Code RCU The Summit*/
# Các file config được đẩy vào qua đường wifi AcessPoint 192.168.4.1
- File config Static Ip cho RCU gồm các thoogn tin IP, Getway, Subnet cấu truc file:
    + Tên mặc định file: "ipconfig.txt"
    + Cấu trúc bên trong file:
        IP: <ip>.<ip>.<ip>.<ip>
        GW: <ip>.<ip>.<ip>.<ip>
        SUB: <ip>.<ip>.<ip>.<ip>

        ex: IP: 192.168.0.159
            GW: 192.168.0.1
            SUB: 255.255.255.0

- File Cấu hình đầu vào đầu ra của RCU các nút nhấn đầu vào và đầu ra tải relay, led, ...
    + Tên mặc định file: "mqtt_in_out.txt"
    + Cấu trúc bên trong file: theo format JSON
        {
            "input":[
                "name_topic",
                "name_topic",
                "name_topic",
                    .
                    .
                    .
                "name_topic",
                "name_topic"
            ],
            "output":[
                "name_topic",
                "name_topic",
                "name_topic",
                    .
                    .
                    .
                "name_topic"
            ]
        }
    ex: 
            {
                "input":[
                    "input1",
                    "input2",
                    "input3",
                    "input4",
                    "mb-input-23-3"
                ],
                "output":[
                    "output1",
                    "output2",
                    "output3",
                    "mb-output-23-3"
                ]
            }
    *NOTE: link mô tả các name_topic
    "https://docs.google.com/spreadsheets/d/1O72LL6_noILVc-vDLNoF1dV2lM1Yiy3d/edit#gid=189076391"
# Thư viện bổ sung vào trong Project
- Sparkplug sử dụng endode/decode gói tin payload trước khi publish MQTT

# 