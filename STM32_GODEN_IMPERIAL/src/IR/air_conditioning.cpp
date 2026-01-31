#include <Arduino.h>
#include "IRremoteESP8266.h"
#include "air_conditioning.h"
#include "IRsend.h"
#include "src/Modbus/modbus_rcu.h"

#ifdef SUMMIT_MITSUBISHI
#include "IRMitsubishiHeavyIndustries.h"
#endif

#ifdef SUMMIT_DAIKIN
#include "ir_Daikin.h"
#endif

void controlAirConditioning(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature)
{

#ifdef SUMMIT_MITSUBISHI
  controlHVACMitsubishi(channel, power, fan, mode, temperature);
#endif

#ifdef SUMMIT_DAIKIN
  controlHVACDaikin(channel, power, fan, mode, temperature);
#endif

}

#ifdef SUMMIT_MITSUBISHI
void controlHVACMitsubishi(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature)
{
  uint16_t irLed;
  switch (channel)
  {
    case CHANNEL_1:
      irLed = CHANNEL_PIN_1;
      break;

    case CHANNEL_2:
      irLed = CHANNEL_PIN_2;
      break;
  }

  IRMitsubishiHeavyIndustries ac(irLed, false, true);
  mapParamIr((uint16_t*) &power,(uint16_t*) &fan,(uint16_t*) &mode,(uint16_t*) &temperature);
  ac.begin();
  ac.setPower(power);
  ac.setFan(fan);
  ac.setMode(mode);
  ac.setTemp(temperature);
  ac.setUDLouverPosition(RCN_UD_AUTO);
  ac.send();

// #ifdef DEBUG_AIR_CONDITIONING
//   writeRS();
//   mySerial.print("power = 0x"); mySerial.println((uint16_t)*power, HEX);
//   mySerial.print("fan = 0x"); mySerial.println((uint16_t)*fan, HEX);
//   mySerial.print("mode = 0x"); mySerial.println((uint16_t)*mode, HEX);
//   mySerial.print("temperature = 0x"); mySerial.println((uint16_t)*temperature, HEX);
//   readRS();
// #endif
}
#endif

#ifdef SUMMIT_DAIKIN
void controlHVACDaikin(uint16_t channel, uint16_t power, uint16_t fan, uint16_t mode, uint16_t temperature)
{
  uint16_t irLed;
  switch (channel)
  {
    case CHANNEL_1:
      irLed = CHANNEL_PIN_1;
      break;

    case CHANNEL_2:
      irLed = CHANNEL_PIN_2;
      break;
  }

  IRDaikin176 ac(irLed);
  mapParamIr((uint16_t*) &power,(uint16_t*) &fan,(uint16_t*) &mode,(uint16_t*) &temperature);
  ac.begin();
  ac.setPower(power);
  ac.setFan(fan);
  ac.setMode(mode);
  ac.setTemp(temperature);
  ac.setSwingHorizontal(kDaikin176SwingHAuto);
  ac.send();

// #ifdef DEBUG_AIR_CONDITIONING
//   writeRS();
//   mySerial.print("power = 0x"); mySerial.println(power, HEX);
//   mySerial.print("fan = 0x"); mySerial.println(fan, HEX);
//   mySerial.print("mode = 0x"); mySerial.println(mode, HEX);
//   mySerial.print("temperature = 0x"); mySerial.println(temperature, HEX);
//   readRS();
// #endif

}
#endif

void mapParamIr(uint16_t* power, uint16_t* fan, uint16_t* mode, uint16_t* temperature)
{

#ifdef DEBUG_AIR_CONDITIONING
  writeRS();
  mySerial.print("power = 0x"); mySerial.println((uint16_t)*power, HEX);
  mySerial.print("fan = 0x"); mySerial.println((uint16_t)*fan, HEX);
  mySerial.print("mode = 0x"); mySerial.println((uint16_t)*mode, HEX);
  mySerial.print("temperature = 0x"); mySerial.println((uint16_t)*temperature, HEX);
  readRS();
#endif

  for (uint8_t i = 0; i < NUMB_STT_POWER; i++)
  {
    if (*power == mapPower[0][i])
    {
      *power = mapPower[1][i];
      break;
    }
  }

  for (uint8_t i = 0; i < NUMB_STT_FAN; i++)
  {
    if (*fan == mapFan[0][i])
    {
      *fan = mapFan[1][i];
      break;
    }
  }

  for (uint8_t i = 0; i < NUMB_STT_MODE; i++)
  {
    if (*mode == mapMode[0][i])
    {
      *mode = mapMode[1][i];
      break;
    }
  }

#ifdef DEBUG_AIR_CONDITIONING
  writeRS();
  mySerial.print("power = 0x"); mySerial.println((uint16_t)*power, HEX);
  mySerial.print("fan = 0x"); mySerial.println((uint16_t)*fan, HEX);
  mySerial.print("mode = 0x"); mySerial.println((uint16_t)*mode, HEX);
  mySerial.print("temperature = 0x"); mySerial.println((uint16_t)*temperature, HEX);
  readRS();
#endif
}
