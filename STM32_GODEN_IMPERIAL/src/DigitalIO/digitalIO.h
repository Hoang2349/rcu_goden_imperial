/* pin_input.h
  Chuc nang bao gom:
    + Tra ve gia tri nut nhan
	+ Dieu khien cong output

    ref: https://www.nxp.com/docs/en/data-sheet/PCA9532.pdf
*/

#ifndef DIGITALIO_h
#define DIGITALIO_h

#include <arduino.h>

#define READ_ERROR 0xFFFF
#define ERROR 0x00
#define SUCCESS 0x01
#define FAILSE 0x00
#define PASS 0x01

#define PCA9532_ADDR_LED_OUT_FIRST   0x60
#define PCA9532_ADDR_LED_OUT_SECOND  0x61
#define PCA9532_ADDR_IN_FIRST   0x62
#define PCA9532_ADDR_IN_SECOND  0x63
#define PCA9532_ADDR_OUPUT  0x64
#define SDA PB7
#define SCL PB6

#define LS0 0b00000110          //LED0 to LED3 selector
#define LS1 0b00000111          //LED4 to LED7 selector
#define LS2 0b00001000          //LED8 to LED11 selector
#define LS3 0b00001001          //LED12 to LED15 selector

#define INPUT0 0b00000000       //input register 0
#define INPUT1 0b00000001       //input register 1

#define OFF_ALL 0b00000000      //off all pin in register of PCA9532
#define NA

#define ON 0xFF00
#define OFF 0x0000
#define BLINK 0x5500

#define CTRL_ON 0x01
#define CTRL_OFF 0x00

#define NUMB_INPUT_MODULE_FIRST        15
#define NUMB_INPUT_MODULE_SECOND       15

#define NUMB_NUM_LED_BER_LS 4

#define NUMB_LED_OUTPUT_MODULE_FIRST        15
#define NUMB_LED_OUTPUT_MODULE_SECOND       12

#define MAX_CYCLE_AC_HALF 10    //f=50Hz -> t=20ms. Using TLP280 -> Zero detect at 1/2 cycle = 10ms. 
#define NUMB_OF_TRIAC 10
#define NUMB_OF_RELAY 15

#define ZERO_DETECT_PIN PA0

#define ADD_BIT_START 0b00001000
#define ADD_BIT_START_POSITION 12

#define MAX_POINTER_BLINK 20

#define NUMB_SENSOR 10

#define TIMEOUT_CHECK_SENSOR 1000

#define TIME_RELAY_ONOFF 10

//////////////////// Enable - Disable debug
// #define DEBUG_SENSOR
// #define DEBUG_ADD_SENSOR
// #define DEBUG_CONTROL_OUTPUT

static uint8_t arrSensor[NUMB_SENSOR] = {0};
static uint32_t cycleUpdateStatus[NUMB_SENSOR] = {0};

typedef struct blink_task_t {
  uint8_t pinout;
  uint8_t ledout;
  uint8_t statusPinout;
  uint8_t statusLedout;
  uint16_t timmer;
  uint32_t timePoint;
};
static struct blink_task_t* blink_task[MAX_POINTER_BLINK];

typedef struct
{
  uint8_t outputRelay;
  uint8_t outputTriac;
  uint8_t outputStatus;
  uint32_t timePointRelayOnoff;
} digital_output_t;
static digital_output_t* digital_output_p;


enum buttonNumber {
  BUTTON1	 = 1,
  BUTTON2	 = 2,
  BUTTON3	 = 3,
  BUTTON4	 = 4,
  BUTTON5	 = 5,
  BUTTON6	 = 6,
  BUTTON7	 = 7,
  BUTTON8	 = 8,
  BUTTON9	 = 9,
  BUTTON10 = 10,
  BUTTON11 = 11,
  BUTTON12 = 12,
  BUTTON13 = 13,
  BUTTON14 = 14,
  BUTTON15 = 15,
  BUTTON16 = 16,
  BUTTON17 = 17,
  BUTTON18 = 18,
  BUTTON19 = 19,
  BUTTON20 = 20,
  BUTTON21 = 21,
  BUTTON22 = 22,
  BUTTON23 = 23,
  BUTTON24 = 24,
  BUTTON25 = 25,
  BUTTON26 = 26,
  BUTTON27 = 27
};

enum LedNotfNumber {    //led notification
  LED_NOTF1	 = 1,
  LED_NOTF2	 = 2,
  LED_NOTF3	 = 3,
  LED_NOTF4	 = 4,
  LED_NOTF5	 = 5,
  LED_NOTF6	 = 6,
  LED_NOTF7	 = 7,
  LED_NOTF8	 = 8,
  LED_NOTF9	 = 9,
  LED_NOTF10 = 10,
  LED_NOTF11 = 11,
  LED_NOTF12 = 12,
  LED_NOTF13 = 13,
  LED_NOTF14 = 14,
  LED_NOTF15 = 15,
  LED_NOTF16 = 16,
  LED_NOTF17 = 17,
  LED_NOTF18 = 18,
  LED_NOTF19 = 19,
  LED_NOTF20 = 20,
  LED_NOTF21 = 21,
  LED_NOTF22 = 22,
  LED_NOTF23 = 23,
  LED_NOTF24 = 24,
  LED_NOTF25 = 25,
  LED_NOTF26 = 26,
  LED_NOTF27 = 27,
  LED_NOTF28 = 28,
  LED_NOTF29 = 29,
  LED_NOTF30 = 30
};

enum addReg {
  ADD_A0 = LED_NOTF28,
  ADD_A1 = LED_NOTF29,
  ADD_A2 = LED_NOTF30
};

enum inputReg {
  LED0 	= 0b0000000000000001,
  LED1 	= 0b0000000000000010,
  LED2 	= 0b0000000000000100,
  LED3 	= 0b0000000000001000,
  LED4 	= 0b0000000000010000,
  LED5 	= 0b0000000000100000,
  LED6 	= 0b0000000001000000,
  LED7 	= 0b0000000010000000,
  LED8	= 0b0000000100000000,
  LED9    = 0b0000001000000000,
  LED10   = 0b0000010000000000,
  LED11   = 0b0000100000000000,
  LED12   = 0b0001000000000000,
  LED13   = 0b0010000000000000,
  LED14   = 0b0100000000000000,
  LED15   = 0b1000000000000000
};

enum outputRelayNumber {    //ouput
  OUTPUT_RELAY1	 = 1,
  OUTPUT_RELAY2	 = 2,
  OUTPUT_RELAY3	 = 3,
  OUTPUT_RELAY4	 = 4,
  OUTPUT_RELAY5	 = 5,
  OUTPUT_RELAY6	 = 6,
  OUTPUT_RELAY7	 = 7,
  OUTPUT_RELAY8	 = 8,
  OUTPUT_RELAY9	 = 9,
  OUTPUT_RELAY10 = 10,
  OUTPUT_RELAY11 = 11,
  OUTPUT_RELAY12 = 12,
  OUTPUT_RELAY13 = 13,
  OUTPUT_RELAY14 = 14,
  OUTPUT_RELAY15 = 15,
};

enum outputTriacNumber {    //ouput
  OUTPUT_TRIAC1	 = 1,
  OUTPUT_TRIAC2	 = 2,
  OUTPUT_TRIAC3	 = 3,
  OUTPUT_TRIAC4	 = 4,
  OUTPUT_TRIAC5	 = 5,
  OUTPUT_TRIAC6	 = 6,
  OUTPUT_TRIAC7	 = 7,
  OUTPUT_TRIAC8	 = 8,
  OUTPUT_TRIAC9	 = 9,
  OUTPUT_TRIAC10	 = 10
};

enum outputledReg {
  LS_LED0 	= 0b00000001,
  LS_LED1 	= 0b00000100,
  LS_LED2 	= 0b00010000,
  LS_LED3 	= 0b01000000,
};

const static uint16_t markPinInputRegFirst = 0b0111111111111111;
static uint8_t oldStatusButtonFirst[NUMB_INPUT_MODULE_FIRST] = {0};

static uint16_t arrLinkBitToPinModuleFirst[2][NUMB_INPUT_MODULE_FIRST] = {
  {LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10, LED11, LED12, LED13, LED14},
  /*IN3_RJ2, IN2_RJ2, IN1_RJ2, IN1_RJ3, IN2_RJ3, IN3_RJ3, IN1_RJ4, IN2_RJ4, IN3_RJ4, IN3_RJ5, IN2_RJ5, IN1_RJ5, IN1_RJ6, IN2_RJ6, IN3_RJ6,
       |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |   ,
      4-2   ,  5-2   ,  6-2   ,  6-3   ,  5-3   ,  4-3   ,  6-4   ,  5-4   ,  4-4   ,  4-6   ,  5-6   ,  6-6   ,  6-5   ,  5-5   ,  4-5  ,
       1    ,   2    ,   3    ,   6    ,   5    ,   4    ,   9    ,   8    ,   7    ,   13   ,   14   ,   15   ,   12   ,   11   ,   10  ,
  */
  {BUTTON1, BUTTON2, BUTTON3, BUTTON6, BUTTON5, BUTTON4, BUTTON9, BUTTON8, BUTTON7, BUTTON13, BUTTON14, BUTTON15, BUTTON12, BUTTON11, BUTTON10}
};


const static uint16_t markPinInputRegSecond = 0b0000111111111111;
const static uint16_t markPinAddRegSecond = 0b0111000000000000;
static uint8_t oldStatusButtonSecond[NUMB_INPUT_MODULE_SECOND] = {0};

static uint16_t arrLinkBitToPinModuleSecond[2][NUMB_INPUT_MODULE_SECOND] = {
  {LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10, LED11, LED12, LED13, LED14},
  /*IN1_RJ7, IN2_RJ7, IN3_RJ7, IN1_RJ8, IN2_RJ8, IN3_RJ8, IN3_RJ9, IN2_RJ9, IN1_RJ9, IN3_RJ10, IN2_RJ10, IN1_RJ10
       |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,   |    ,    |    ,    |
      6-7   ,  5-7   ,  4-7   ,  6-8   ,  5-8   ,  4-8   ,  4-9   ,  5-9   ,  6-9   ,  4-10   ,   5-10   ,  6-10
  */
  {BUTTON18, BUTTON17, BUTTON16, BUTTON21, BUTTON20, BUTTON19, BUTTON22, BUTTON23, BUTTON24, BUTTON25, BUTTON26, BUTTON27, ADD_A0, ADD_A1, ADD_A2}
};

//-------------------------------Module Led First-------------------------------//

static uint8_t linkLedPinToBitModuleLS0First[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT3_RJ3, OUT3_RJ2, OUT1_RJ2, OUT2_RJ2
       |    ,   |    ,   |    ,   |
      7-3   ,  7-2   ,  1-2   ,  2-2
  */
  {LED_NOTF4, LED_NOTF1, LED_NOTF3, LED_NOTF2}
};

static uint8_t linkLedPinToBitModuleLS1First[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT3_RJ4, OUT1_RJ4, OUT1_RJ3, OUT2_RJ3
       |    ,   |    ,   |    ,   |
      7-4   ,  1-4   ,  1-3   ,  2-3
  */
  {LED_NOTF7, LED_NOTF9, LED_NOTF6, LED_NOTF5}
};

static uint8_t linkLedPinToBitModuleLS2First[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT1_RJ6, OUT2_RJ6, OUT3_RJ6, OUT2_RJ4
       |    ,   |    ,   |    ,   |
      1-6   ,  2-6   ,  7-6   ,  2-4
  */
  {LED_NOTF12, LED_NOTF11, LED_NOTF10, LED_NOTF8}
};

static uint8_t linkLedPinToBitModuleLS3First[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED2, LS_LED1, LS_LED0, 0},
  /*OUT1_RJ5, OUT2_RJ5, OUT3_RJ5
       |    ,   |    ,   |
      1-5   ,  2-5   ,  7-5
  */
  {LED_NOTF15, LED_NOTF14, LED_NOTF13, 0}
};

//-------------------------------Module led Second-------------------------------//

static uint8_t linkLedPinToBitModuleLS0Second[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT3_RJ8, OUT3_RJ7, OUT2_RJ7, OUT1_RJ7
       |    ,   |    ,   |    ,   |
      7-8   ,  7-7   ,  2-7   ,  1-7
  */
  {LED_NOTF19, LED_NOTF16, LED_NOTF17, LED_NOTF18}
};

static uint8_t linkLedPinToBitModuleLS1Second[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT1_RJ9, OUT2_RJ9, OUT1_RJ8, OUT2_RJ8
       |    ,   |    ,   |    ,   |
      1-9   ,  2-9   ,  1-8   ,  2-8
  */
  {LED_NOTF24, LED_NOTF23, LED_NOTF21, LED_NOTF20}
};

static uint8_t linkLedPinToBitModuleLS2Second[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  /*OUT1_RJ10, OUT2_RJ10, OUT3_RJ10, OUT3_RJ9
       |    ,   |    ,   |    ,   |
      1-10  ,  2-10  ,  7-10  ,  7-9
  */
  {LED_NOTF27, LED_NOTF26, LED_NOTF25, LED_NOTF22}
};

//-------------------------------Module Output-------------------------------//

static uint8_t linkOutputPinToBitModuleLS0[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  {OUTPUT_RELAY5, OUTPUT_RELAY6, OUTPUT_RELAY7, OUTPUT_RELAY8}
};

static uint8_t linkOutputPinToBitModuleLS1[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  {OUTPUT_RELAY1, OUTPUT_RELAY2, OUTPUT_RELAY3, OUTPUT_RELAY4}
};

static uint8_t linkOutputPinToBitModuleLS2[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED3, LS_LED2, LS_LED1, LS_LED0},
  {OUTPUT_RELAY12, OUTPUT_RELAY11, OUTPUT_RELAY10, OUTPUT_RELAY9}
};

static uint8_t linkOutputPinToBitModuleLS3[2][NUMB_NUM_LED_BER_LS] = {
  {LS_LED2, LS_LED1, LS_LED0, 0},
  {OUTPUT_RELAY15, OUTPUT_RELAY14, OUTPUT_RELAY13, 0}
};

/*PCB Ver6*/
// static uint8_t linkOutputPinTriacToPin[][NUMB_OF_TRIAC] = {
//   {PB4, PB8, PB2, PB0, PA6, PA4, PA5, PA7, PB1, PA8},
//   {OUTPUT_TRIAC1, OUTPUT_TRIAC2, OUTPUT_TRIAC3, OUTPUT_TRIAC4, OUTPUT_TRIAC5, OUTPUT_TRIAC6, OUTPUT_TRIAC7, OUTPUT_TRIAC8, OUTPUT_TRIAC9, OUTPUT_TRIAC10}
// };

/*PCB Ver9*/
static uint8_t linkOutputPinTriacToPin[][NUMB_OF_TRIAC] = {
  {PB8, PA4, PB4, PA6, PA5, PB0, PA7, PB1, PB2, PA8},
  {OUTPUT_TRIAC1, OUTPUT_TRIAC2, OUTPUT_TRIAC3, OUTPUT_TRIAC4, OUTPUT_TRIAC5, OUTPUT_TRIAC6, OUTPUT_TRIAC7, OUTPUT_TRIAC8, OUTPUT_TRIAC9, OUTPUT_TRIAC10}
};

// ============hard code for Goden Imperial===========//
#define DOOR_SENSOR_PIN (8)
#define MOTION_SENSOR_PIN (7)
enum doorSensorStatus_t {
  DOOR_SENSOR_OPEN = 0,
  DOOR_SENSOR_CLOSE = 1
};

typedef struct doorSensor_t {
  bool doorSensorStatus_cur;
  bool doorSensorStatus_new;
} doorSensor_t;

enum motionSensorStatus_t {
  MOTION_SENSOR_ACTIVE = 0,
  MOTION_SENSOR_INACTIVE = 1
};

typedef struct motionSensor_t {
  bool motionSensorStatus_cur;
  bool motionSensorStatus_new;
} motionSensor_t;


static doorSensor_t doorSensor = {DOOR_SENSOR_CLOSE, DOOR_SENSOR_CLOSE};
static motionSensor_t motionSensor = {MOTION_SENSOR_INACTIVE, MOTION_SENSOR_INACTIVE};

uint8_t initButton();
uint8_t getClickButton();
uint16_t scanModuleBtnFirst();
uint16_t scanModuleBtnSecond();
void initInputModuleFirst();
void initInputModuleSecond();
void initI2C();
uint8_t getListTriggerPinModuleFirst(uint16_t inputBtnReg);
uint8_t getListTriggerPinModuleSecond(uint16_t inputBtnReg);
uint8_t scanRegBtn(uint8_t inputReg, uint8_t address);
void writeOutputReg(uint8_t reg, uint8_t value, uint8_t address);

void controlLed(uint8_t ledNum, uint8_t status);
uint8_t getLedReg(uint8_t ledNum);
uint8_t getLedBitCtrl(uint8_t ledNum, uint8_t status);
uint8_t linkLedPinToBit(uint8_t ledNum, uint8_t linkLedPinToBitArr[][4]);

uint8_t controlOutput(uint8_t pinout, uint8_t status);
void controlTriac(uint8_t pinout, uint8_t status);
void controlRelay(uint8_t pinout, uint8_t status);
uint8_t getOuputReg(uint8_t pinout);
uint8_t getOutputBitCtrl(uint8_t pinout, uint8_t status);
uint8_t linkPinoutRelayToBit(uint8_t pinout, uint8_t linkPinoutToBitArr[][4]);
uint8_t  linkPinoutTriacToPin(uint8_t pinout, uint8_t linkPinoutToPinArr[][10]);
uint8_t getZeroDetect();
void initPinout();

uint8_t getLedStatus(uint8_t ledNum);
uint8_t getOutputStatus(uint8_t pinout);
uint8_t getMyAddress();

void updateStatusPinModuleFirst(uint16_t inputBtnReg);
void updateStatusPinModuleSecond(uint16_t inputBtnReg);

void additionalSensor(uint16_t sensor_pin);

uint8_t checkTimeoutSensor(uint32_t *p_cycleUpdateStatus);
void updateTimeoutSensor(uint32_t *p_cycleUpdateStatus);

void update_ouput_status();

uint8_t getFreqZeroDetech();
uint8_t printFreqZeroDetech();
#endif
