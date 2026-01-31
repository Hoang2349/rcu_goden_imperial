#include "src/DigitalIO/digitalIO.h"
#include "src/Dimming/dimming.h"
#include "src/Modbus/modbus_rcu.h"
#include "src/IR/air_conditioning.h"

// #define RESPONSE
#define PRINT_FREQ_ZD
//#define DEBUG_DELAY_BUTTON

uint32_t timeOutCheckButton = 100;
uint32_t timePointCheckButton = 0;

void setup()
{
  initButton();
  initPinout();
  setupDimming();
  modbusSetup();
  my_addr = getMyAddress();
#ifdef DEBUG_ADDR
  debugAddr();
#endif

#ifdef PRINT_FREQ_ZD
  printFreqZeroDetech();
#endif
}

void loop()
{

  //------------------------ Buttoon event ------------------------//
  uint8_t evenButton = getButtonStatus();
  if (evenButton != NO)
  {
    sentEventButtonModbus(evenButton);
#ifdef DEBUG_DELAY_BUTTON
    writeRS();
    mySerial.println("sbt"); // send button
    readRS();
#endif
  }

  //------------------------ Receive from Master ------------------------//
  uint8_t *arr = (uint8_t *)malloc(MAX_DATA_A_PACKET);
  uint8_t sizeof_dta = 0;
  if (checkModbus(arr, &sizeof_dta) == YES)
  {
    modbus_rcu *packet = (modbus_rcu *)malloc(sizeof(modbus_rcu));
    uint8_t stt = handle_packet(arr, &sizeof_dta, packet);
    if (packet->addr == my_addr)
    {
      switch (packet->func)
      {
      case PINOUT_CTR:
        handle_pinout(packet);
        break;

      case DIMMING:
        handle_dimming(packet);
        break;

      case AIR_CONDITIONER:
        handle_air_coner(packet);
        break;

      case ADD_SENSOR:
        handle_additional_sensor(packet);
        break;
      }
#ifdef RESPONSE
      responseModbus(arr, sizeof_dta);
#endif
    }
    free(packet);
  }
  free(arr);
  hiden_task();
}

/**
   @brief

   @param packet
*/
void handle_pinout(modbus_rcu *packet)
{
#ifdef DEBUG_DELAY_BUTTON
  writeRS();
  mySerial.println("rbt"); // receive button
  readRS();
#endif
  output_t *output = (output_t *)malloc(sizeof(output_t));
  parse_pinout_packet(output, packet);
  uint8_t status;
  if (output->value == ON)
  {
    status = CTRL_ON;
  }

  if (output->value == OFF)
  {
    status = CTRL_OFF;
  }
  if (output->timmer == NORMAL)
  {
    controlLed(output->ledout, status);
    controlOutput(output->pinout, status);
    #ifdef DEBUG_DELAY_BUTTON
    writeRS();
    mySerial.println("cod");    // control output done
    readRS();
#endif
  }
  else
  {
    for (uint8_t i = 0; i < MAX_POINTER_BLINK; i++)
    {
      if (blink_task[i] == NULL)
      {
        blink_task[i] = (blink_task_t *)malloc(sizeof(blink_task_t));
        blink_task[i]->pinout = output->pinout;
        blink_task[i]->ledout = output->ledout;
        // blink_task[i]->statusPinout = (getOutputStatus(blink_task[i]->pinout) != CTRL_OFF ? CTRL_ON : CTRL_OFF);   
        // blink_task[i]->statusLedout = (getLedStatus(blink_task[i]->ledout) != CTRL_OFF ? CTRL_ON : CTRL_OFF);
        blink_task[i]->statusPinout = !status;    //Hieu edit fix bug curtain
        blink_task[i]->statusLedout = !status;
        blink_task[i]->timePoint = millis();
        blink_task[i]->timmer = output->timmer;

        controlLed(blink_task[i]->ledout, status);
        controlOutput(blink_task[i]->pinout, status);
        break;
      }
    }
  }

  free(output);
}

/**
   @brief

   @param packet
*/
void handle_dimming(modbus_rcu *packet)
{
  dimming_t *dimming = (dimming_t *)malloc(sizeof(dimming_t));
  parse_dimming_packet(dimming, packet);
  controlDimming(dimming->channel, dimming->value);
  free(dimming);
}

void handle_air_coner(modbus_rcu *packet)
{
  air_condiotioner_t *air_condiotioner = (air_condiotioner_t *)malloc(sizeof(air_condiotioner_t));
  parse_air_coner_packet(air_condiotioner, packet);
  controlAirConditioning(air_condiotioner->channel, air_condiotioner->power, air_condiotioner->fan, air_condiotioner->mode, air_condiotioner->temperature);
  free(air_condiotioner);
}

void handle_additional_sensor(modbus_rcu *packet)
{
  sensor_t *sensor = (sensor_t *)malloc(sizeof(sensor_t));
  parse_sensor_packet(sensor, packet);
  additionalSensor(sensor->sensor_pin);
  free(sensor);
}

void hiden_task()
{
  for (uint8_t i = 0; i < MAX_POINTER_BLINK; i++)
  {
    if (blink_task[i] != NULL)
    {
      uint32_t timeTemp = 0;
      if (millis() >= blink_task[i]->timePoint)
      {
        timeTemp = millis() - blink_task[i]->timePoint;
      }
      else
      {
        timeTemp = blink_task[i]->timePoint - millis();
      }

      if (timeTemp > blink_task[i]->timmer)
      {
        controlLed(blink_task[i]->ledout, blink_task[i]->statusLedout);
        controlOutput(blink_task[i]->pinout, blink_task[i]->statusPinout);
        free(blink_task[i]);
        blink_task[i] = NULL;
      }
    }
  }

  update_ouput_status();
}

uint8_t getButtonStatus()
{
  uint32_t timeTemp = 0;
  if (millis() >= timePointCheckButton)
  {
    timeTemp = millis() - timePointCheckButton;
  }
  else
  {
    timeTemp = timePointCheckButton - millis();
  }

  if (timeTemp > timeOutCheckButton)
  {
    timePointCheckButton = millis();
    return getClickButton();
  }
  return NO;
}
