#ifndef MODBUS_RCU_H
#define MODBUS_RCU_H
#include <Arduino.h>
#include <SoftwareSerial.h>

extern uint8_t my_addr;
extern SoftwareSerial mySerial;


#define SERIAL_RX_BUFFER_SIZE (1024)    // redefine 1024byte buffer - about 64 packet control IO (16byte/packet).

#define OK 1
#define YES 1
#define NO 0
#define NORMAL 0
#define MAX_DATA 40
#define MAX_DATA_A_PACKET (40 + 8)  // 1byte addr + 1byte func + 2byte col + 1byte qtt + 1byte byte count + 2byte CRC + data
#define MAX_SIZEOF_BUFFER_UART 1024
#define TIME_OUT_UART 100    //9600bit/s = 1200byte/1000ms -> 1024byte = 854ms;
#define LEN_PACKET_BUTTON 8
#define HEADER_LEN 5 // 1byte addr + 1byte func + 2byte col + 1byte qtt

#define txPin PA9
#define rxPin PA10
#define enRS PC6
// #define DEBUG
//#define DEBUG_ADDR
//#define DEBUG_IO
//#define DEBUG_ADD_SENSOR
//#define DEBUG_MODBUS
//#define DEBUG_AIR_CONDITIONING
// #define DEBUG_CONTROL_TRIAC
// #define DEBUG_PACKET

#define PRINT_FREQ_ZD

#define BAURATE 9600
//#define BAURATE 19200
//#define BAURATE 38400
//#define BAURATE 56000
//#define BAURATE 115200

#define CRC16_MODBUS 0xA001
#define INIT_CHECKSUM 0xFFFF
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)


//----------------func----------------//
enum func_value
{
  PINOUT_CTR          = 0X10,
  DIMMING             = 0X06,
  AIR_CONDITIONER     = 0x05,
  BUTTON              = 0X04,
  ADD_SENSOR          = 0X07
};

//----------------register----------------//
#define REG_OUTPUT 0x0001;
#define REG_DIMMING 0x0004;
#define REG_BUTTON 0X0002;

enum REG_AIR_CONDITIONER
{
  AIR_CON_POWER   = 0X0101,
  AIR_CON_TEMP    = 0X0102,
  AIR_CON_MODE    = 0X0103,
  WIND            = 0X0104
};



#include <arduino.h>

struct packet_receive_t
{
  uint8_t size = 0;
  char data[MAX_DATA_A_PACKET] = {0};
};


struct modbus_rcu
{
  uint8_t addr;
  uint8_t func;
  uint16_t col_reg_addr;
  uint8_t quantity_reg;
  uint8_t byte_count;
  uint8_t data[MAX_DATA];
  uint16_t crc;
};

struct output_t
{
  uint16_t pinout;
  uint16_t ledout;
  uint16_t value;
  uint16_t timmer;

};

struct dimming_t
{
  uint16_t channel;
  uint16_t value;
};

struct air_condiotioner_t
{
  uint16_t channel;
  uint16_t power;
  uint16_t fan;
  uint16_t mode;
  uint16_t temperature;
};

struct packet_event_button_t
{
  uint8_t arr[LEN_PACKET_BUTTON];
  uint8_t address;
  uint8_t func;
  uint16_t coil;
  uint16_t button;
  uint16_t crc;
};

struct sensor_t
{
  uint16_t sensor_pin;
};

struct freq_zero_detech_t
{
  uint16_t freq;
};

static struct packet_event_button_t packet_event_button;

void modbusSetup();
uint8_t checkModbus(uint8_t* arr, uint8_t* sizeof_dta);
void responseModbus(uint8_t* arr, uint8_t sizeof_dta);
uint8_t handle_packet(uint8_t* arr, uint8_t* sizeof_dta, modbus_rcu* packet );
uint8_t parse_pinout_packet(output_t* output, modbus_rcu* packet);
uint8_t parse_dimming_packet(dimming_t* dimming, modbus_rcu* packet);
uint8_t parse_air_coner_packet(air_condiotioner_t* air_condiotioner, modbus_rcu* packet);
uint8_t parse_sensor_packet(sensor_t* sensor, modbus_rcu* packet);
void sentEventButtonModbus(uint8_t button);
uint16_t calCRC16(uint8_t *buff, int len);
void fillCRC(uint8_t *buff, int len);
void debugAddr();
void writeRS();
void readRS();
void printLog(const String log);
void printLogln(const String log);


//handle function for golden imperial
void sentEventDoorSensor(uint8_t status);
void sentEventMotionSensor(uint8_t status);

#endif
