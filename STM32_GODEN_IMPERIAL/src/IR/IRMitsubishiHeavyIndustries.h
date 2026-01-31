/*
 * IRMitsubishiHeavyIndustries.h
 *
 *  Created on: Jan 17, 2019
 *      Author: RD001
 */
//HieuNV: Add code for IR Mitsubishi Arduino IDE - START
#include <Arduino.h>
#include "IRsend.h"
//HieuNV: Add code for IR Mitsubishi Arduino IDE - END


#ifndef LIBRARIES_IR_IRMITSUBISHIHEAVYINDUSTRIES_H_
#define LIBRARIES_IR_IRMITSUBISHIHEAVYINDUSTRIES_H_


//HieuNV: Add code for IR Mitsubishi Arduino IDE - START
#define MITSUBISHI_HEAVY_INDUSTRIES_PACKET_SIZE         20U
//HieuNV: Add code for IR Mitsubishi Arduino IDE - END


#define MITSUBISHI_HEAVY_INDUSTRIES_AC_HDR_MARK         6000U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_HDR_SPACE        7500U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_BIT_MARK         550U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_ONE_SPACE        3450U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_ZERO_SPACE       1450U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FOOTER_MARK      480U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_GAP_SPACE        7200U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_STOP_BIT_SPACE   7200U

// Constants
//#define MITSUBISHI_HEAVY_INDUSTRIES_AC_AUTO           0x20U
//#define MITSUBISHI_HEAVY_INDUSTRIES_AC_COOL           0x18U
//#define MITSUBISHI_HEAVY_INDUSTRIES_AC_DRY            0x10U
//#define MITSUBISHI_HEAVY_INDUSTRIES_AC_HEAT           0x08U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_POWER          0x01

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_AUTO       0b00100000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_1          0b00000000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_2          0b10000000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_3          0b01000000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_4          0b11000000

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MIN_FAN        0U  // Auto
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MAX_FAN        4U  // Speed 4

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_AUTO      0b00000000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_COOL      0b00000100
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_HEAT      0b00000010
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_DRY       0b00001000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MODE_FAN       0b00001100

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_Auto        0b00001100 // not sure
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_1           0b00000000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_2           0b00001000
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_3           0b00000100
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_UD_4           0b00001100

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_FAN_SILENT        6U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MIN_TEMP         18U  // 16C
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_MAX_TEMP         30U  // 31C

#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_18           0b01000000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_19           0b11000000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_20           0b00100000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_21           0b10100000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_22           0b01100000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_23           0b11100000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_24           0b00010000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_25           0b10010000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_26           0b01010000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_27           0b11010000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_28           0b00110000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_29           0b10110000
#define MITSUBISHI_HEAVY_INDUSTRIES_TEMP_30           0b01110000

#define MITSUBISHI_HEAVY_INDUSTRIES_AC_VANE_AUTO         0U
#define MITSUBISHI_HEAVY_INDUSTRIES_AC_VANE_AUTO_MOVE    7U
enum MHI_RCN_FAN_SPEED
{
    RCN_FAN_AUTO = 0U,
    RCN_FAN_1   = 1U,
    RCN_FAN_2   = 2U,
    RCN_FAN_3   = 3U,
    RCN_FAN_4   = 4U
};



enum MHI_RCN_UD
{
    RCN_UD_AUTO = 0,
    RCN_UD_1    = 1, // Horizontal
    RCN_UD_2    = 2,
    RCN_UD_3    = 3,
    RCN_UD_4    = 4 // Vertical
};
class IRMitsubishiHeavyIndustries//:  public IR
{
public:
    uint8_t remote_state[MITSUBISHI_HEAVY_INDUSTRIES_PACKET_SIZE];
    void checksum();

    IRMitsubishiHeavyIndustries(const uint16_t pin, const bool inverted,const bool use_modulation);
    void stateReset();
    void send();
    void begin();
    void on();
    void off();
    void setPower(bool state);
    bool getPower();
    void setTemp(uint8_t temp);
    uint8_t getTemp();
    void setFan(uint8_t fan);
    uint8_t getFan();
    void setMode(uint8_t mode);
    uint8_t getMode();
    void setUDLouverPosition(uint8_t pos);
    uint8_t getUDLouverPosition();
    void setSilent(bool state);
    bool getSilent();
    void setECO(bool state);
    bool getECO();
    void setHighPower(bool state);
    bool getHighPower();

    uint8_t* getRaw();

    virtual ~IRMitsubishiHeavyIndustries();

private:
    IRsend _irsend;  ///< instance of the IR send class
};

#endif /* LIBRARIES_IR_IRMITSUBISHIHEAVYINDUSTRIES_H_ */
