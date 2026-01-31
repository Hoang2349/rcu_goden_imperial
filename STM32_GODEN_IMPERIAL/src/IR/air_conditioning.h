/**
   @file hvac_mitsubishi.h
   @author hieunv (hieunv@elifetech.vn)
   @brief ham dieu khien dieu hoa hang Mitsubishi.
   Ref:
   + https://gitlab.com/elife-technology/hardware/rcu-v2/-/tree/example_stm32_ir/driver/IRremoteESP8266
   + https://github.com/crankyoldgit/IRremoteESP8266

   @version 0.1
   @date 2022-10-03

   @copyright Copyright (c) 2022

*/

#ifndef AIR_CONDITIONING_h
#define AIR_CONDITIONING_h

#include <arduino.h>
#include "ir_Daikin.h"
#include "IRMitsubishiHeavyIndustries.h"


#define CHANNEL_PIN_1 PA2
#define CHANNEL_PIN_2 PA3

#ifdef ESP8266
#define CHANNEL_PIN_1 4
#define CHANNEL_PIN_2 5
#endif

#define NUMB_STT_POWER  2
#define NUMB_STT_FAN    3
#define NUMB_STT_MODE   5

//Define type of air conditioning here
#define SUMMIT_MITSUBISHI
//#define SUMMIT_DAIKIN


//#define DEBUG_AIR_CONDITIONING

enum ir_channed
{
  CHANNEL_1 = 1,
  CHANNEL_2 = 2
};

enum power {
  POWER_ON    = 0x01,
  POWER_OFF   = 0x00
};

enum mode {
  Auto        = 0,
  Cool        = 1,
  Dry         = 2,
  Fan         = 3,
  Heat        = 4
};

enum fan {
  FanAuto  = 0x00,   //additional case
  FanLow   = 0x01,
  FanMed   = 0x02,
  FanHigh  = 0x03,
  FanMax   = 0x04,   //additional case
  FanEcono = 0x06,   //additional case
  FanTurbo = 0x08    //additional case
};

enum swingVertical {
  SwingVAuto       = 0,
  SwingVHighest    = 1,
  SwingVHigh       = 2,
  SwingVMiddle     = 3,
  SwingVLow        = 4,
  SwingVLowest     = 5,
  SwingVOff        = 6
};

enum setSwingHorizontal {
  SwingHAuto 		= 0,
  SwingHLeftMax 	= 1,
  SwingHLeft 		= 2,
  SwingHMiddle 	= 3,
  SwingHRight 		= 4,
  SwingHRightMax 	= 5,
  SwingHRightLeft 	= 6,
  SwingHLeftRight 	= 7,
  SwingHOff 		= 8
};

void mapParamIr(uint16_t* power, uint16_t* fan, uint16_t* mode, uint16_t* temperature);
void controlAirConditioning(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature);

#ifdef SUMMIT_MITSUBISHI
#include "IRMitsubishiHeavyIndustries.h"

#define IR_TEMP_MIN  18
#define IR_TEMP_MAX  30

//power
#define IR_POWER_ON   0x01
#define IR_POWER_OFF  0x00

//fan
#define IR_FAN_LOW   RCN_FAN_1
#define IR_FAN_MED   RCN_FAN_2
#define IR_FAN_HIGH  RCN_FAN_4

//mode
#define IR_AUTO     AC_MODE_AUTO
#define IR_COOL     AC_MODE_COOL
#define IR_DRY      AC_MODE_DRY
#define IR_FAN      AC_MODE_FAN
#define IR_HEAT     AC_MODE_HEAT

void controlHVACMitsubishi(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature);
#endif

#ifdef SUMMIT_DAIKIN
#include "ir_Daikin.h"
#define IR_TEMP_MIN  18
#define IR_TEMP_MAX  32

//power
#define IR_POWER_ON   0x01
#define IR_POWER_OFF  0x00

//fan
#define IR_FAN_LOW   kDaikinFanMin
#define IR_FAN_MED   kDaikinFanMed
#define IR_FAN_HIGH  kDaikinFanMax

//mode
#define IR_AUTO     kDaikin176Auto
#define IR_COOL     kDaikin176Cool
#define IR_DRY      kDaikin176Dry
#define IR_FAN      kDaikin176Fan

void controlHVACDaikin(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature);
#endif

static uint16_t mapPower[2][NUMB_STT_POWER] =
{
  {POWER_ON, POWER_OFF},
  {IR_POWER_ON, IR_POWER_OFF}
};

static uint16_t mapFan[2][NUMB_STT_FAN] =
{
  {FanLow, FanMed, FanHigh},
  {IR_FAN_LOW, IR_FAN_MED, IR_FAN_HIGH}
};

static uint16_t mapMode[2][NUMB_STT_MODE] =
{
  {Auto, Cool, Dry, Fan, Heat},
  {IR_AUTO, IR_COOL, IR_DRY, IR_FAN, IR_HEAT}
};
#endif
