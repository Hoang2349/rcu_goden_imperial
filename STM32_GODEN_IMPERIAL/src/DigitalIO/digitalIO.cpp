#include "digitalIO.h"
#include "../Modbus/modbus_rcu.h"
#include "../Queue/queue.h"
#include <Wire.h>
uint8_t initButton()
{
  initI2C();
  initInputModuleFirst();
  initInputModuleSecond();
  cleanQueueButton();
  cleanQueueControlTriac();
}

/*--------Read button-----------------------------------------------------------------*/

uint8_t getClickButton()
{
  uint16_t btnTempFirst = 0x0000;
  uint16_t btnTempSecond = 0x0000;
  btnTempFirst = scanModuleBtnFirst();

  // handle door sensor Goden Imperial
  if ((btnTempFirst & LED8) == 0)
  {
    doorSensor.doorSensorStatus_new = DOOR_SENSOR_OPEN;
  }
  else
  {
    doorSensor.doorSensorStatus_new = DOOR_SENSOR_CLOSE;
  }
  if (doorSensor.doorSensorStatus_cur != doorSensor.doorSensorStatus_new)
  {
    doorSensor.doorSensorStatus_cur = doorSensor.doorSensorStatus_new;
    sentEventDoorSensor(doorSensor.doorSensorStatus_cur);
  }

  //handle morion sensor
  if ((btnTempFirst & LED7) == 0)
  {
    motionSensor.motionSensorStatus_new = MOTION_SENSOR_INACTIVE;
  }
  else
  {
    motionSensor.motionSensorStatus_new = MOTION_SENSOR_ACTIVE;
  }
  if (motionSensor.motionSensorStatus_cur != motionSensor.motionSensorStatus_new)
  {
    motionSensor.motionSensorStatus_cur = motionSensor.motionSensorStatus_new;
    sentEventMotionSensor(motionSensor.motionSensorStatus_cur);
  }

  // readRS();
  // end

  btnTempSecond = scanModuleBtnSecond();
  uint16_t btnTempFirstOrg = btnTempFirst;
  uint16_t btnTempSecondOrg = btnTempSecond;

  if (~btnTempFirst == READ_ERROR)
  {
    return READ_ERROR;
  }
  else
  {
    btnTempFirst = ~btnTempFirst & markPinInputRegFirst;
    if ((btnTempFirst) != 0)
    {
#ifdef DEBUG_SENSOR
      writeRS();
      mySerial.print("btnTempFirstOrg = ");
      mySerial.println(btnTempFirstOrg, BIN);
      // mySerial.print(", btnTempFirstOrg = ");
      // mySerial.println(btnTempFirstOrg, BIN);
      readRS();
#endif
      getListTriggerPinModuleFirst(btnTempFirst);
    }
  }

  if (~btnTempSecond == READ_ERROR)
  {
    return READ_ERROR;
  }
  else
  {
    btnTempSecond = ~btnTempSecond & markPinInputRegSecond;
    if ((btnTempSecond) != 0)
    {
#ifdef DEBUG_SENSOR
      writeRS();
      mySerial.print("btnTempSecondOrg = ");
      mySerial.println(btnTempSecondOrg, BIN);
      // mySerial.print("btnTempSecondOrg = ");
      // mySerial.println(btnTempSecondOrg, BIN);
      readRS();
#endif
      getListTriggerPinModuleSecond(btnTempSecond);
    }
  }

  updateStatusPinModuleFirst(btnTempFirst);
  updateStatusPinModuleSecond(btnTempSecond);

  if (isEmptyQueueButton() == FALSE)
  {
    uint8_t tmpBtn = peekQueueButton();
    deQueueButton();
    return tmpBtn;
  }
  return 0;
}
uint16_t scanModuleBtnFirst()
{
  uint16_t tempReg = 0x0000;
  tempReg = scanRegBtn(INPUT1, PCA9532_ADDR_IN_FIRST);
  tempReg = tempReg << 8;
  tempReg += scanRegBtn(INPUT0, PCA9532_ADDR_IN_FIRST);
  return tempReg;
}
uint16_t scanModuleBtnSecond()
{
  uint16_t tempReg = 0x0000;
  tempReg = scanRegBtn(INPUT1, PCA9532_ADDR_IN_SECOND);
  tempReg = tempReg << 8;
  tempReg += scanRegBtn(INPUT0, PCA9532_ADDR_IN_SECOND);
  return tempReg;
}

void initInputModuleFirst()
{
  writeOutputReg(PCA9532_ADDR_IN_FIRST, LS0, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_FIRST, LS1, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_FIRST, LS2, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_FIRST, LS3, OFF_ALL);
}

void initInputModuleSecond()
{
  writeOutputReg(PCA9532_ADDR_IN_SECOND, LS0, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_SECOND, LS1, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_SECOND, LS2, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_IN_SECOND, LS3, OFF_ALL);
}

void initI2C()
{
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin();
}

void writeOutputReg(uint8_t reg, uint8_t value, uint8_t address)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t getListTriggerPinModuleFirst(uint16_t inputBtnReg)
{

  for (int i = 0; i < NUMB_INPUT_MODULE_FIRST; i++)
  {
    // handle door sensor Goden Imperial
    if(i == DOOR_SENSOR_PIN || i == MOTION_SENSOR_PIN)
    {
      continue;
    }
    // end

    if (inputBtnReg & arrLinkBitToPinModuleFirst[0][i])
    {
      uint8_t isSensor = FALSE;

      for (uint8_t t = 0; t < NUMB_SENSOR; t++) // duyet danh sach cam bien
      {
        if (arrLinkBitToPinModuleFirst[1][i] == arrSensor[t])
        {
          isSensor = TRUE;
          if (checkTimeoutSensor(&cycleUpdateStatus[t]) == TRUE)
          {
#ifdef DEBUG_SENSOR
            writeRS();
            mySerial.print("First: i = ");
            mySerial.print(t);
            mySerial.print(", ");
            mySerial.print("cycleUpdateStatus = ");
            mySerial.println(cycleUpdateStatus[t]);
            readRS();
#endif
            enQueueButton((uint8_t)arrLinkBitToPinModuleFirst[1][i]);
            updateTimeoutSensor(&cycleUpdateStatus[t]);
            break;
          }
        }
      }

      if (isSensor == FALSE)
      {
        if (oldStatusButtonFirst[i] == CTRL_OFF)
        {
          enQueueButton((uint8_t)arrLinkBitToPinModuleFirst[1][i]);
        }
      }
#ifdef DEBUG_IO
      writeRS();
      mySerial.print("-------------------------------------");
      mySerial.print("- first [");
      mySerial.print(i);
      mySerial.print("] = ");
      mySerial.println(inputBtnReg & arrLinkBitToPinModuleFirst[0][i]);
      mySerial.print("- old[");
      mySerial.print(i);
      mySerial.print("] = ");
      mySerial.println(oldStatusButtonFirst[i]);
      mySerial.print("-------------------------------------");
      readRS();
#endif
    }
  }
  return TRUE;
}

uint8_t getListTriggerPinModuleSecond(uint16_t inputBtnReg)
{
  for (int i = 0; i < NUMB_INPUT_MODULE_SECOND; i++)
  {
    if (inputBtnReg & arrLinkBitToPinModuleSecond[0][i])
    {
      uint8_t isSensor = FALSE;
      for (uint8_t t = 0; t < NUMB_SENSOR; t++) // duyet danh sach cam bien
      {
        if (arrLinkBitToPinModuleSecond[1][i] == arrSensor[t])
        {
          isSensor = TRUE;
          if (checkTimeoutSensor(&cycleUpdateStatus[t]) == TRUE)
          {
#ifdef DEBUG_SENSOR
            writeRS();
            mySerial.print("Second : i = ");
            mySerial.print(t);
            mySerial.print(", ");
            mySerial.print("cycleUpdateStatus = ");
            mySerial.println(cycleUpdateStatus[t]);
            readRS();
#endif
            enQueueButton((uint8_t)arrLinkBitToPinModuleSecond[1][i]);
            updateTimeoutSensor(&cycleUpdateStatus[t]);
            break;
          }
        }
      }

      if (isSensor == FALSE)
      {
        if (oldStatusButtonSecond[i] == CTRL_OFF)
        {
          enQueueButton((uint8_t)arrLinkBitToPinModuleSecond[1][i]);
        }
      }
    }
  }
  return TRUE;
}

uint8_t scanRegBtn(uint8_t inputReg, uint8_t address)
{
  uint8_t response = 0x00;
  Wire.beginTransmission(address);
  Wire.write(inputReg);
  Wire.endTransmission();

  Wire.requestFrom(address, 1);
  while (Wire.available())
  {
    response = Wire.read();
  }
  return response;
}

/*--------Control led out-----------------------------------------------------------------*/
void controlLed(uint8_t ledNum, uint8_t status)
{
  if ((0 < ledNum) && (ledNum <= NUMB_LED_OUTPUT_MODULE_FIRST))
  {
    uint8_t regTemp = getLedReg(ledNum);
    uint8_t bitCtrlTemp = getLedBitCtrl(ledNum, status);
    writeOutputReg(regTemp, bitCtrlTemp, PCA9532_ADDR_LED_OUT_FIRST);
    return;
  }

  if (ledNum <= (NUMB_LED_OUTPUT_MODULE_FIRST + NUMB_LED_OUTPUT_MODULE_SECOND))
  {
    uint8_t regTemp = getLedReg(ledNum);
    uint8_t bitCtrlTemp = getLedBitCtrl(ledNum, status);
    writeOutputReg(regTemp, bitCtrlTemp, PCA9532_ADDR_LED_OUT_SECOND);
    return;
  }
  return;
}

uint8_t getLedReg(uint8_t ledNum)
{

  int i = 0;
  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS0First[1][i] == ledNum)
    {
      return (uint8_t)LS0;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS1First[1][i] == ledNum)
    {
      return (uint8_t)LS1;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS2First[1][i] == ledNum)
    {
      return (uint8_t)LS2;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS3First[1][i] == ledNum)
    {
      return (uint8_t)LS3;
    }
  }

  //-------------------------------------------------
  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS0Second[1][i] == ledNum)
    {
      return (uint8_t)LS0;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS1Second[1][i] == ledNum)
    {
      return (uint8_t)LS1;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkLedPinToBitModuleLS2Second[1][i] == ledNum)
    {
      return (uint8_t)LS2;
    }
  }
}

uint8_t getLedBitCtrl(uint8_t ledNum, uint8_t status)
{
  if ((ledNum == LED_NOTF1) || (ledNum == LED_NOTF2) || (ledNum == LED_NOTF3) || (ledNum == LED_NOTF4))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_LED_OUT_FIRST);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0First)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0First)));
    }
    return regTemp;
  }

  if ((ledNum == LED_NOTF5) || (ledNum == LED_NOTF6) || (ledNum == LED_NOTF7) || (ledNum == LED_NOTF9))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_LED_OUT_FIRST);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1First)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1First)));
    }
    return regTemp;
  }

  if ((ledNum == LED_NOTF8) || (ledNum == LED_NOTF10) || (ledNum == LED_NOTF11) || (ledNum == LED_NOTF12))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_LED_OUT_FIRST);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2First)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2First)));
    }
    return regTemp;
  }

  if ((ledNum == LED_NOTF13) || (ledNum == LED_NOTF14) || (ledNum == LED_NOTF15))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS3, PCA9532_ADDR_LED_OUT_FIRST);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS3First)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS3First)));
    }
    return regTemp;
  }
  //-------------------------------Second
  if ((ledNum == LED_NOTF16) || (ledNum == LED_NOTF17) || (ledNum == LED_NOTF18) || (ledNum == LED_NOTF19))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_LED_OUT_SECOND);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0Second)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0Second)));
    }
    return regTemp;
  }

  if ((ledNum == LED_NOTF20) || (ledNum == LED_NOTF21) || (ledNum == LED_NOTF23) || (ledNum == LED_NOTF24))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_LED_OUT_SECOND);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1Second)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1Second)));
    }
    return regTemp;
  }

  if ((ledNum == LED_NOTF22) || (ledNum == LED_NOTF25) || (ledNum == LED_NOTF26) || (ledNum == LED_NOTF27))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_LED_OUT_SECOND);
    if (status)
    {
      regTemp = (regTemp | (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2Second)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2Second)));
    }
    return regTemp;
  }
}

uint8_t linkLedPinToBit(uint8_t ledNum, uint8_t linkLedPinToBitArr[][4])
{
  for (int i = 0; i < 4; i++)
  {
    if (ledNum == linkLedPinToBitArr[1][i])
    {
      return linkLedPinToBitArr[0][i];
    }
  }
}

/*--------Control Output-----------------------------------------------------------------*/

uint8_t controlOutput(uint8_t pinout, uint8_t status)
{

  /**
   * Bat relay -> cho den khi relay bat han -> bat triac
   * Buoc 1: bat relay
   * Buoc 2: luu thoi diem bat relay
   * Buoc 3: them triac vao hang doi    //enQueueControlTriac(digital_output_p)
   * Buoc 4: kiem tra da qua timeout de bat triac + zero detech chua, neu da qua timeout
   * Buoc 5: bat triac
   * Buoc 6: xoa hang doi triac va timeout
   */

  digital_output_p = (digital_output_t *)malloc(sizeof(digital_output_t));
  digital_output_p->outputRelay = pinout;
  digital_output_p->outputTriac = pinout;
  digital_output_p->outputStatus = status;

  if ((pinout > 0) && (pinout <= NUMB_OF_RELAY))
  {
    controlRelay(pinout, status);
    digital_output_p->timePointRelayOnoff = millis();
#ifdef DEBUG_CONTROL_OUTPUT
    writeRS();
    mySerial.print("Relay: pinout = ");
    mySerial.print(pinout);
    mySerial.print(", status = ");
    mySerial.println(status);
    readRS();
#endif
  }

  if ((pinout > 0) && pinout <= NUMB_OF_TRIAC)
  {
    enQueueControlTriac(digital_output_p);
    digital_output_p->timePointRelayOnoff = millis();
#ifdef DEBUG_CONTROL_OUTPUT
    writeRS();
    mySerial.print("Triac: pinout = ");
    mySerial.print(pinout);
    mySerial.print(", status = ");
    mySerial.println(status);
    readRS();
#endif
  }
  free(digital_output_p);
  return SUCCESS;
}

void controlTriac(uint8_t pinout, uint8_t status)
{
  if (pinout == 0)
  {
    return;
  }
  pinout = linkPinoutTriacToPin(pinout, linkOutputPinTriacToPin);
  pinMode(pinout, OUTPUT);
  digitalWrite(pinout, status);
}

void controlRelay(uint8_t pinout, uint8_t status)
{
  int8_t regTemp = getOuputReg(pinout);
  uint8_t bitCtrlTemp = getOutputBitCtrl(pinout, status);
  writeOutputReg(regTemp, bitCtrlTemp, PCA9532_ADDR_OUPUT);
  return;
}

uint8_t getOuputReg(uint8_t pinout)
{

  int i = 0;
  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkOutputPinToBitModuleLS0[1][i] == pinout)
    {
      return (uint8_t)LS0;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkOutputPinToBitModuleLS1[1][i] == pinout)
    {
      return (uint8_t)LS1;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkOutputPinToBitModuleLS2[1][i] == pinout)
    {
      return (uint8_t)LS2;
    }
  }

  for (int i = 0; i < NUMB_NUM_LED_BER_LS; i++)
  {
    if (linkOutputPinToBitModuleLS3[1][i] == pinout)
    {
      return (uint8_t)LS3;
    }
  }
}

uint8_t getOutputBitCtrl(uint8_t pinout, uint8_t status)
{
  if ((pinout == OUTPUT_RELAY8) || (pinout == OUTPUT_RELAY7) || (pinout == OUTPUT_RELAY6) || (pinout == OUTPUT_RELAY5))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_OUPUT);
    if (status)
    {
      regTemp = (regTemp | (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS0)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS0)));
    }
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY4) || (pinout == OUTPUT_RELAY3) || (pinout == OUTPUT_RELAY2) || (pinout == OUTPUT_RELAY1))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_OUPUT);
    if (status)
    {
      regTemp = (regTemp | (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS1)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS1)));
    }
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY9) || (pinout == OUTPUT_RELAY10) || (pinout == OUTPUT_RELAY11) || (pinout == OUTPUT_RELAY12))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_OUPUT);
    if (status)
    {
      regTemp = (regTemp | (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS2)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS2)));
    }
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY13) || (pinout == OUTPUT_RELAY14) || (pinout == OUTPUT_RELAY15))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS3, PCA9532_ADDR_OUPUT);
    if (status)
    {
      regTemp = (regTemp | (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS3)));
    }

    if (!status)
    {
      regTemp = (regTemp & (~linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS3)));
    }
    return regTemp;
  }
}

uint8_t linkPinoutRelayToBit(uint8_t pinout, uint8_t linkPinoutToBitArr[][4])
{
  for (int i = 0; i < 4; i++)
  {
    if (pinout == linkPinoutToBitArr[1][i])
    {
      return linkPinoutToBitArr[0][i];
    }
  }
}

uint8_t linkPinoutTriacToPin(uint8_t pinout, uint8_t linkPinoutToPinArr[][10])
{
  for (int i = 0; i < NUMB_OF_TRIAC; i++)
  {
    if (pinout == linkPinoutToPinArr[1][i])
    {
      return linkPinoutToPinArr[0][i];
    }
  }
}

uint8_t getZeroDetect()
{
  pinMode(ZERO_DETECT_PIN, INPUT);
  if (digitalRead(ZERO_DETECT_PIN) == HIGH)
  {
    return PASS;
  }
  return FAILSE;
}

void initPinout()
{
  // initI2C();
  writeOutputReg(PCA9532_ADDR_OUPUT, LS0, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_OUPUT, LS1, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_OUPUT, LS2, OFF_ALL);
  writeOutputReg(PCA9532_ADDR_OUPUT, LS3, OFF_ALL);
}

uint8_t getLedStatus(uint8_t ledNum)
{
  if ((ledNum == LED_NOTF1) || (ledNum == LED_NOTF2) || (ledNum == LED_NOTF3) || (ledNum == LED_NOTF6))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_LED_OUT_FIRST);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0First)));
    return regTemp;
  }

  if ((ledNum == LED_NOTF4) || (ledNum == LED_NOTF5) || (ledNum == LED_NOTF8) || (ledNum == LED_NOTF9))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_LED_OUT_FIRST);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1First)));
    return regTemp;
  }

  if ((ledNum == LED_NOTF7) || (ledNum == LED_NOTF13) || (ledNum == LED_NOTF14) || (ledNum == LED_NOTF15))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_LED_OUT_FIRST);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2First)));
    return regTemp;
  }

  if ((ledNum == LED_NOTF10) || (ledNum == LED_NOTF11) || (ledNum == LED_NOTF12))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS3, PCA9532_ADDR_LED_OUT_FIRST);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS3First)));
    return regTemp;
  }
  //-------------------------------Second
  if ((ledNum == LED_NOTF16) || (ledNum == LED_NOTF17) || (ledNum == LED_NOTF18) || (ledNum == LED_NOTF21))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_LED_OUT_SECOND);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS0Second)));
    return regTemp;
  }

  if ((ledNum == LED_NOTF19) || (ledNum == LED_NOTF20) || (ledNum == LED_NOTF23) || (ledNum == LED_NOTF24))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_LED_OUT_SECOND);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS1Second)));
    return regTemp;
  }

  if ((ledNum == LED_NOTF22) || (ledNum == LED_NOTF25) || (ledNum == LED_NOTF26) || (ledNum == LED_NOTF27))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_LED_OUT_SECOND);
    regTemp = (regTemp & (linkLedPinToBit(ledNum, linkLedPinToBitModuleLS2Second)));
    return regTemp;
  }
}

uint8_t getOutputStatus(uint8_t pinout)
{
  if ((pinout == OUTPUT_RELAY8) || (pinout == OUTPUT_RELAY7) || (pinout == OUTPUT_RELAY6) || (pinout == OUTPUT_RELAY5))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS0, PCA9532_ADDR_OUPUT);
    regTemp = (regTemp & (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS0)));
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY4) || (pinout == OUTPUT_RELAY3) || (pinout == OUTPUT_RELAY2) || (pinout == OUTPUT_RELAY1))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS1, PCA9532_ADDR_OUPUT);
    regTemp = (regTemp & (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS1)));
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY9) || (pinout == OUTPUT_RELAY10) || (pinout == OUTPUT_RELAY11) || (pinout == OUTPUT_RELAY12))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS2, PCA9532_ADDR_OUPUT);
    regTemp = (regTemp & (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS2)));
    return regTemp;
  }

  if ((pinout == OUTPUT_RELAY13) || (pinout == OUTPUT_RELAY14) || (pinout == OUTPUT_RELAY15))
  {
    uint8_t regTemp = 0x00;
    regTemp = scanRegBtn(LS3, PCA9532_ADDR_OUPUT);
    regTemp = (regTemp & (linkPinoutRelayToBit(pinout, linkOutputPinToBitModuleLS3)));
    return regTemp;
  }
}

uint8_t getMyAddress()
{
  uint16_t btnTemp = 0x0000;
  btnTemp = scanModuleBtnSecond();
  if (~btnTemp == READ_ERROR)
  {
    return READ_ERROR;
  }
  else
  {
    btnTemp = ~btnTemp & markPinAddRegSecond;
    uint8_t my_addr_temp = (btnTemp >> ADD_BIT_START_POSITION) + ADD_BIT_START;
    return my_addr_temp;
  }
}

void updateStatusPinModuleFirst(uint16_t inputBtnReg)
{
  for (uint8_t i = 0; i < NUMB_INPUT_MODULE_FIRST; i++)
  {
    if (inputBtnReg & arrLinkBitToPinModuleFirst[0][i])
    {
      oldStatusButtonFirst[i] = CTRL_ON;
    }
    else
    {
      oldStatusButtonFirst[i] = CTRL_OFF;
    }
#ifdef DEBUG_IO
    writeRS();
    mySerial.print("update old[");
    mySerial.print(i);
    mySerial.print("] = ");
    mySerial.println(oldStatusButtonFirst[i]);
    readRS();
#endif
  }
}

void updateStatusPinModuleSecond(uint16_t inputBtnReg)
{
  for (uint8_t i = 0; i < NUMB_INPUT_MODULE_SECOND; i++)
  {
    if (inputBtnReg & arrLinkBitToPinModuleSecond[0][i])
    {
      oldStatusButtonSecond[i] = CTRL_ON;
    }
    else
    {
      oldStatusButtonSecond[i] = CTRL_OFF;
    }
  }
}

void additionalSensor(uint16_t sensor_pin)
{
  for (uint8_t i = 0; i < NUMB_SENSOR; i++)
  {
    if (arrSensor[i] == 0)
    {
      arrSensor[i] = sensor_pin;
      cycleUpdateStatus[i] = 0; // Hieu edit bug
#ifdef DEBUG_ADD_SENSOR
      writeRS();
      mySerial.print("i = ");
      mySerial.print(i);
      mySerial.print(", add sensor pin = ");
      mySerial.println(sensor_pin);
      readRS();
#endif
      return;
    }
  }
}

uint8_t checkTimeoutSensor(uint32_t *p_cycleUpdateStatus)
{
  uint32_t timeTemp = 0;
  if (millis() >= *p_cycleUpdateStatus)
  {
    timeTemp = millis() - *p_cycleUpdateStatus;
  }
  else
  {
    timeTemp = *p_cycleUpdateStatus - millis();
  }

  //// proccess
  if (timeTemp >= TIMEOUT_CHECK_SENSOR)
  {
    return TRUE;
  }

  return FALSE;
}

void updateTimeoutSensor(uint32_t *p_cycleUpdateStatus)
{
  *p_cycleUpdateStatus = millis();
}

void update_ouput_status()
{
  digital_output_t *output_temp = peekQueueControlTriac();
  // if((output_temp != NULL) && (output_temp->outputTriac != 0))
  // {
  if (output_temp != NULL)
  {
    if (((millis() - output_temp->timePointRelayOnoff) > TIME_RELAY_ONOFF) && (getZeroDetect() == PASS))
    {

#ifdef DEBUG_CONTROL_TRIAC
      writeRS();
      mySerial.print("Triac on = ");
      mySerial.print(output_temp->outputTriac, DEC);
      mySerial.print(" - ");
      mySerial.print(output_temp->outputStatus, DEC);
      mySerial.println();
      readRS();
#endif

      controlTriac(output_temp->outputTriac, output_temp->outputStatus);
      deQueueControlTriac();
    }
  }
}

uint8_t getFreqZeroDetech()
{
  pinMode(ZERO_DETECT_PIN, INPUT);
  uint32_t timeStart = millis();
  uint8_t counter = 0;
  uint8_t update = true;

  while (millis() - timeStart < 1000)
  {
    if ((digitalRead(ZERO_DETECT_PIN) == HIGH) && (update == true))
    {
      counter++;
      update = false;
    }
    if (digitalRead(ZERO_DETECT_PIN) == LOW)
    {
      update = true;
    }
  }
  return counter;
}

uint8_t printFreqZeroDetech()
{
  uint8_t freq = getFreqZeroDetech();
  writeRS();
  mySerial.print("Frequence of Zero Detech = ");
  mySerial.println(freq);
  readRS();
  return true;
}