/* break;
 * IRMitsubishiHeavyIndustries.cpp
 *
 *  Created on: Jan 17, 2019
 *      Author: RD001
 */

#include "IRMitsubishiHeavyIndustries.h"
#include "IRutils.h"
#include "IRsend.h"



IRMitsubishiHeavyIndustries::IRMitsubishiHeavyIndustries(const uint16_t pin, 
const bool inverted,const bool use_modulation) : _irsend(pin, inverted, use_modulation)
{}

IRMitsubishiHeavyIndustries::~IRMitsubishiHeavyIndustries()
{
    // TODO Auto-generated destructor stub
}

//HieuNV: Add code for IR Mitsubishi Arduino IDE - START
void IRsend::sendMitsubishiHeavyIndustriesAC(unsigned char data[], uint16_t nbytes,
                              uint16_t repeat) {
  if (nbytes < MITSUBISHI_HEAVY_INDUSTRIES_PACKET_SIZE)
    return;  // Not enough bytes to send a proper message.

  sendGeneric(MITSUBISHI_HEAVY_INDUSTRIES_AC_HDR_MARK, MITSUBISHI_HEAVY_INDUSTRIES_AC_HDR_SPACE,
              MITSUBISHI_HEAVY_INDUSTRIES_AC_BIT_MARK, MITSUBISHI_HEAVY_INDUSTRIES_AC_ONE_SPACE,
              MITSUBISHI_HEAVY_INDUSTRIES_AC_BIT_MARK, MITSUBISHI_HEAVY_INDUSTRIES_AC_ZERO_SPACE,
              MITSUBISHI_HEAVY_INDUSTRIES_AC_FOOTER_MARK, MITSUBISHI_HEAVY_INDUSTRIES_AC_GAP_SPACE,
              data, nbytes, 38, true, repeat, 50);

  mark(MITSUBISHI_HEAVY_INDUSTRIES_AC_BIT_MARK);
}

void IRMitsubishiHeavyIndustries::begin()
{
    _irsend.begin();
    stateReset();
}
//HieuNV: Add code for IR Mitsubishi Arduino IDE - END

// Reset the state of the remote to a known good state/sequence.
void IRMitsubishiHeavyIndustries::stateReset()
{
//    remote_state[0] = 0xB0;
//    remote_state[1] = 0x00;
//    remote_state[2] = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25|MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_COOL|0; //25 oC, Cool Mode, PowerOff
//    remote_state[3] = 0x0|MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_2|0;   // notsure| U/D position2 | notsure
//
//    remote_state[4] = ~remote_state[0];  // invert of byte 0
//    remote_state[5] = ~remote_state[1];  // invert of byte 1
//    remote_state[6] = ~remote_state[2];  // invert of byte 2
//    remote_state[7] = ~remote_state[3];  // invert of byte 3
//
//    remote_state[8] = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO;           // Fan speed Auto
//    remote_state[9] = 0x80;           // silent off
//    remote_state[10] = 0x28;          // highpower off, eco off
//    remote_state[11] = 0x00;
//    remote_state[12] = ~remote_state[8];             // invert of byte 8
// 0  1  2  3   4  5  6  7   8  9  10 11  12 13 14 15  16 17 18 19
// B0 00 45 04  4F FF BA FB  20 80 28 00  DF 7F D7 FF  40 BF 00 00

    remote_state[0] = 0xB0;
    remote_state[1] = 0x00;
    remote_state[2] = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25|MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_COOL|0; //25 oC, Cool Mode, PowerOff
    remote_state[3] = 0x0|MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_2|0;   // notsure| U/D position2 | notsure

    remote_state[4] = ~remote_state[0];  // invert of byte 0
    remote_state[5] = ~remote_state[1];  // invert of byte 1
    remote_state[6] = ~remote_state[2];  // invert of byte 2
    remote_state[7] = ~remote_state[3];  // invert of byte 3

    remote_state[8]  = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO;
    remote_state[9]  = 0x80;           // silent off
    remote_state[10] = 0x28;          // highpower off, eco off
    remote_state[11] = 0x00;

    remote_state[12] = ~remote_state[8];  // invert of byte 8
    remote_state[13] = ~remote_state[9];  // invert of byte 9
    remote_state[14] = ~remote_state[10];  // invert of byte 10
    remote_state[15] = ~remote_state[11];  // invert of byte 11

    remote_state[16] = 0x40;
    remote_state[17] = 0xBF;
    remote_state[18] = 0x00;
    remote_state[19] = 0x00;

}

// Send the current desired state to the IR LED.
//HieuNV: Add code for IR Mitsubishi Arduino IDE - START
void IRMitsubishiHeavyIndustries::send() {
    checksum();   // Ensure correct checksum before sending.
    _irsend.sendMitsubishiHeavyIndustriesAC(remote_state, MITSUBISHI_HEAVY_INDUSTRIES_PACKET_SIZE, false);
}
//HieuNV: Add code for IR Mitsubishi Arduino IDE - END

// Return a pointer to the internal state date of the remote.
uint8_t* IRMitsubishiHeavyIndustries::getRaw() {
  checksum();
  return remote_state;
}

// Calculate the checksum for the current internal state of the remote.
void IRMitsubishiHeavyIndustries::checksum() {
    remote_state[4] = ~remote_state[0];  // invert of byte 0
    remote_state[5] = ~remote_state[1];  // invert of byte 1
    remote_state[6] = ~remote_state[2];  // invert of byte 2
    remote_state[7] = ~remote_state[3];  // invert of byte 3t of byte 8

    remote_state[12] = ~remote_state[8];  // invert of byte 8
    remote_state[13] = ~remote_state[9];  // invert of byte 9
    remote_state[14] = ~remote_state[10];  // invert of byte 10
    remote_state[15] = ~remote_state[11];  // invert of byte 11

}

// Set the requested power state of the A/C to on.
void IRMitsubishiHeavyIndustries::on() {
  // state = ON;
  remote_state[2] |= MITSUBISHI_HEAVY_INDUSTRIES_AC_POWER; //setbit

}

// Set the requested power state of the A/C to off.
void IRMitsubishiHeavyIndustries::off() {
  // state = OFF;
  remote_state[2] &= (~MITSUBISHI_HEAVY_INDUSTRIES_AC_POWER);  // clearbit

}

// Set the requested power state of the A/C.
void IRMitsubishiHeavyIndustries::setPower(bool state) {
  if (state)
    IRMitsubishiHeavyIndustries::on();
  else
    IRMitsubishiHeavyIndustries::off();
}

// Return the requested power state of the A/C.
bool IRMitsubishiHeavyIndustries::getPower() {
  return((remote_state[2] & MITSUBISHI_HEAVY_INDUSTRIES_AC_POWER) !=0 );
}

// Set the temp. in deg C, 18 to 30 only
void IRMitsubishiHeavyIndustries::setTemp(uint8_t temp) {
  temp = max((uint8_t) MITSUBISHI_HEAVY_INDUSTRIES_AC_MIN_TEMP, temp);
  temp = min((uint8_t) MITSUBISHI_HEAVY_INDUSTRIES_AC_MAX_TEMP, temp);
  uint8_t tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25;
  switch (temp)
  {
      case 18: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_18; break;
      case 19: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_19; break;
      case 20: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_20; break;
      case 21: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_21; break;
      case 22: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_22; break;
      case 23: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_23; break;
      case 24: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_24; break;
      case 25: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25; break;
      case 26: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_26; break;
      case 27: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_27; break;
      case 28: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_28; break;
      case 29: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_29; break;
      case 30: tempCode = MITSUBISHI_HEAVY_INDUSTRIES_TEMP_30; break;
      default: break;
  }
    remote_state[2] &= 0b00001111;
    remote_state[2] |= tempCode; // clear first 4bits, set mask
}

// Return the set temp. in deg C
uint8_t IRMitsubishiHeavyIndustries::getTemp()
{
  uint8_t temp = 25;
  uint8_t tempCode = remote_state[2] & 0xF0; // get first 4 bits only

    switch (tempCode)
    {
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_18: temp = 18; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_19: temp = 19; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_20: temp = 20; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_21: temp = 21; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_22: temp = 22; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_23: temp = 23; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_24: temp = 24; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25: temp = 25; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_26: temp = 26; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_27: temp = 27; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_28: temp = 28; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_29: temp = 29; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_TEMP_30: temp = 30; break;
        default: break;
    }

  return(temp);
}

// Set the speed of the fan, 0-4.
// 0 is auto, 1-4 is the speed.
void IRMitsubishiHeavyIndustries::setFan(uint8_t fan) {

    uint8_t fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1;
    switch (fan)
    {
        case RCN_FAN_AUTO:  fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO; break;
        case RCN_FAN_1:     fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1; break;
        case RCN_FAN_2:     fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_2; break;
        case RCN_FAN_3:     fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_3; break;
        case RCN_FAN_4:     fanCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_4; break;
        default: break;
    }
    remote_state[8] = remote_state[8] & 0x0F | fanCode; // clear first 4bits, set mask
}

// Return the requested state of the unit's fan.
uint8_t IRMitsubishiHeavyIndustries::getFan()
{
    uint8_t fan = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1;
    uint8_t fanCode = remote_state[8] & 0xF0; // get first 4 bits only
    switch (fanCode)
    {
        case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO : fanCode = RCN_FAN_AUTO; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1    : fanCode = RCN_FAN_1   ; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_2    : fanCode = RCN_FAN_2   ; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_3    : fanCode = RCN_FAN_3   ; break;
        case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_4    : fanCode = RCN_FAN_4   ; break;
        default: break;
    }
    return(fan);
}

// Set the requested climate operation mode of the a/c unit.
void IRMitsubishiHeavyIndustries::setMode(uint8_t mode)
{
    // mode = max((uint8_t) AC_MODE_AUTO, mode);
    // mode = min((uint8_t) AC_MODE_FAN,  mode);
    uint8_t modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_COOL;
  // If we get an unexpected mode, default to AUTO.
    switch (mode)
    {
        case AC_MODE_AUTO : modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_AUTO; break;
        case AC_MODE_COOL : modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_COOL; break;
        case AC_MODE_HEAT : modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_HEAT; break;
        case AC_MODE_DRY  : modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_DRY ; break;
        case AC_MODE_FAN  : modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_FAN ; break;
        default: modeCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_AUTO; break;
    }
    remote_state[2] &= 0b11110001;  //Clear the previous setting.
    remote_state[2] |= modeCode;
}

// Return the requested climate operation mode of the a/c unit.
uint8_t IRMitsubishiHeavyIndustries::getMode()
{
    uint8_t mode = MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1;
    uint8_t modeCode = remote_state[2] & 0b00001110; // get first 4 bits only, current settings
    switch (modeCode)
    {
       case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO : mode = RCN_FAN_AUTO; break;
       case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1    : mode = RCN_FAN_1   ; break;
       case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_2    : mode = RCN_FAN_2   ; break;
       case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_3    : mode = RCN_FAN_3   ; break;
       case MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_4    : mode = RCN_FAN_4   ; break;
       default: break;
    }
    return(mode);
}

// Set the requested vane operation mode of the a/c unit.
void IRMitsubishiHeavyIndustries::setUDLouverPosition(uint8_t position)
{
    uint8_t positionCode;
  // If we get an unexpected mode, default to AUTO.
    switch (position)
    {
        case RCN_UD_AUTO : positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_Auto; break;
        case RCN_UD_1    : positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_1   ; break;
        case RCN_UD_2    : positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_2   ; break;
        case RCN_UD_3    : positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_3   ; break;
        case RCN_UD_4    : positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_4   ; break;
        default: positionCode = MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_Auto; break;
    }
    remote_state[3] &= 0b11110001;  //Clear the previous setting.
    remote_state[1] &= 0b00000010;  //Clear the previous setting.
    remote_state[3] |= positionCode;
    if(positionCode== MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_Auto)
        remote_state[1] |= 0b00000010;
}

// Return the requested vane operation mode of the a/c unit.
uint8_t IRMitsubishiHeavyIndustries::getUDLouverPosition()
{
  uint8_t position;
  uint8_t positionCode = remote_state[3] & 0b00001110; //
  switch (positionCode)
  {
     case MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_Auto  :
     {
         position = RCN_UD_4;
         if ( (remote_state[1] &=0b00000010)!=0) position = RCN_UD_AUTO;
         break;

     }
     case MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_1     : position = RCN_UD_1   ; break;
     case MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_2     : position = RCN_UD_2   ; break;
     case MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_3     : position = RCN_UD_3   ; break;
     default:  position = RCN_UD_2; break;
  }
  return(position);
}
/*
 * Start or cancel the silent mode control. The silent mode preferentially controls
 * the outdoor unit taking quiet operation into consideration
 * */
void IRMitsubishiHeavyIndustries::setSilent(bool state)
{
    if(state)   remote_state[9] |=0x01;
    else remote_state[9] &= 0xFE;
}
bool IRMitsubishiHeavyIndustries::getSilent()
{
    return (remote_state[9] & 0x01)!=0;
}
/* Start or cancel the energy-saving operation (In the cool, heat, or auto mode only)
 * The energy-saving mode automatically controls the performance depending on the outdoor temerature, setting
 * the standard temperature to 28oC in the cool mode and 22oC in the heat mode.
 * This enables the energy-saving operation without compromising room comfort.
 *
 */
void IRMitsubishiHeavyIndustries::setECO(bool state)
{
    if(state)
    {
        uint8_t mode = getMode();
        switch(mode)
        {
            case AC_MODE_AUTO :
                remote_state[10] |=0x01;
                break;
            case AC_MODE_COOL :
                remote_state[10] |=0x01;
                setTemp(28);
                break;
            case AC_MODE_HEAT :
                remote_state[10] |=0x01;
                setTemp(22);
                break;
            default: break; // for testing only
        }
        remote_state[10] |=0x01;

    }else  remote_state[10] &= 0xFE;
}

bool IRMitsubishiHeavyIndustries::getECO()
{
    return (remote_state[10] & 0x01)!=0;
}
/*
 * Start or cancel the high power operation (In the cool or heat mode only)
 * The high power operation quickly set the room temperature to a comfortable level.
 * This mode accelerates the operation performance for 15mins, and automatically to the normal mode after that
 * */

void IRMitsubishiHeavyIndustries::setHighPower(bool state)
{

}

bool IRMitsubishiHeavyIndustries::getHighPower()
{
    return (remote_state[10] & 0x02)!=0;
}
