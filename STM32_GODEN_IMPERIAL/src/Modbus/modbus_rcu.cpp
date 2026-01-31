#include "modbus_rcu.h"
#include "../DigitalIO/digitalIO.h"
#include <SoftwareSerial.h>
#include <LinkedList.h>

uint8_t my_addr;
LinkedList<packet_receive_t> list;

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

void cpyArray(char *des, char *src, uint16_t pStartReadSrc, uint16_t len);

void modbusSetup()
{
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(enRS, OUTPUT);
  mySerial.begin(BAURATE);
  mySerial.setTimeout(TIME_OUT_UART);

#ifdef DEBUG
  writeRS();
  mySerial.println("Ready for work");
  readRS();
#endif
}

// Define packet  refer: https://docs.google.com/spreadsheets/d/18ThxQxTQZHCnouvGfNcKB5DKbtx0Wumh/edit?usp=sharing&ouid=118437418062467944844&rtpof=true&sd=true
uint8_t checkModbus(uint8_t *arr, uint8_t *sizeof_dta)
{
  packet_receive_t packet_receive;
  bool handlePacket = false;

  char arrTemp[SERIAL_RX_BUFFER_SIZE] = {0};
  uint16_t arrTempLen = 0;
  uint16_t pReadArrTemp = 0;
  char dataHeader[HEADER_LEN] = {0};
  while (mySerial.available()) // nhan duoc tin hieu RS485
  {
    arrTempLen = mySerial.readBytes(arrTemp, SERIAL_RX_BUFFER_SIZE);
  }

  if (arrTempLen > 0)
  {
#ifdef DEBUG_PACKET
    writeRS();
    mySerial.print("Data receive: ");
    for (uint8_t i = 0; i < arrTempLen; i++)
    {
      mySerial.print(arrTemp[i], HEX);
      mySerial.print(' ');
    }
    mySerial.println();
    readRS();
#endif
  }

  while (pReadArrTemp < arrTempLen - 1)
  {
    if (arrTemp[pReadArrTemp] == 0)
    {
      break;
    }
    //-------get header and body-------
    cpyArray(&dataHeader[0], &arrTemp[0], pReadArrTemp, HEADER_LEN);
    pReadArrTemp += HEADER_LEN;
    uint8_t dataBodyLen = dataHeader[4] * 2 + 3; // + 1 byte count + 2 CRC
    char dataBody[dataBodyLen] = {0};
    cpyArray(dataBody, arrTemp, pReadArrTemp, dataBodyLen);
    pReadArrTemp += dataBodyLen;

#ifdef DEBUG_PACKET
    writeRS();
    mySerial.print("dataBodyLen = ");
    mySerial.print(dataBodyLen);
    mySerial.println(", Content: ");
    for (uint8_t i = 0; i < dataBodyLen; i++)
    {
      mySerial.print(dataBody[i], HEX);
      mySerial.print(' ');
    }
    mySerial.println();
    readRS();
#endif

    //-------combine full packet-------
    uint8_t fullPacket[HEADER_LEN + dataBodyLen] = {0};
    memcpy(fullPacket, dataHeader, HEADER_LEN);
    for (uint8_t i = 0; i < dataBodyLen; i++)
    {
      fullPacket[HEADER_LEN + i] = dataBody[i];
    }

    //-------Store packet-------
    packet_receive.size = HEADER_LEN + dataBodyLen;
    memcpy(packet_receive.data, fullPacket, packet_receive.size);

    list.add(packet_receive);
  }

  if (list.size() != 0)
  {
    packet_receive_t packetTmp;
    packetTmp = list.shift();
    cpyArray((char *)arr, packetTmp.data, 0, packetTmp.size);
    *sizeof_dta = packetTmp.size;

    // if(list.size() > 10)
    // {
    //   list.clear();
    // }

    return YES;
  }
  return NO;
}

void cpyArray(char *des, char *src, uint16_t pStartReadSrc, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    *des = *(src + pStartReadSrc);
    des++;
    src++;
  }
}

void responseModbus(uint8_t *arr, uint8_t sizeof_dta)
{
  writeRS();
  for (uint8_t i = 0; i < sizeof_dta; i++)
  {
    mySerial.write(*arr);
    arr++;
  }
  readRS();
}

uint8_t handle_packet(uint8_t *arr, uint8_t *sizeof_dta, modbus_rcu *packet)
{
#ifdef DEBUG
  writeRS();
  mySerial.print("handle, sizeof(modbus_rcu) = ");
  mySerial.println(sizeof(modbus_rcu));
  readRS();
#endif

  packet->addr = *arr;
  arr += sizeof(packet->addr);
  packet->func = *arr;
  arr += sizeof(packet->func);
  packet->col_reg_addr = *arr;
  packet->col_reg_addr = packet->col_reg_addr << 8;
  packet->col_reg_addr += *(arr + 1);
  arr += sizeof(packet->col_reg_addr);
  packet->quantity_reg = *arr;
  arr += sizeof(packet->quantity_reg);
  packet->byte_count = *arr;
  arr += sizeof(packet->byte_count);
  if (packet->byte_count > MAX_DATA)
  {
#ifdef DEBUG
    writeRS();
    mySerial.println("ERROR: out of size");
    readRS();
#endif
    return NO;
  }

  for (uint8_t i = 0; i < packet->byte_count; i++)
  {
    packet->data[i] = *arr;
    arr += 1;
  }
  packet->crc = *arr;
  packet->crc = packet->crc << 8;
  packet->crc += *(arr + 1);

#ifdef DEBUG_MODBUS
  writeRS();
  mySerial.print("addr = 0x");
  mySerial.println(packet->addr, HEX);
  mySerial.print("func = 0x");
  mySerial.println(packet->func, HEX);
  mySerial.print("col_reg_addr = 0x");
  mySerial.println(packet->col_reg_addr, HEX);
  mySerial.print("quantity_reg = 0x");
  mySerial.println(packet->quantity_reg, HEX);
  mySerial.print("byte_count = 0x");
  mySerial.println(packet->byte_count, HEX);
  mySerial.print("data = ");
  for (uint8_t i = 0; i < packet->byte_count; i++)
  {
    mySerial.print("0x");
    mySerial.print(packet->data[i], HEX);
    mySerial.print(',');
  }
  mySerial.println();
  mySerial.print("crc = 0x");
  mySerial.println(packet->crc, HEX);
  mySerial.println("done!");
  readRS();
#endif
  return packet->func;
}

uint8_t parse_pinout_packet(output_t *output, modbus_rcu *packet)
{
  uint8_t index = 0;
  output->pinout = packet->data[index];
  index += sizeof(packet->data[index]);
  output->pinout = output->pinout << 8;
  output->pinout += packet->data[index];
  index += sizeof(packet->data[index]);

  output->ledout = packet->data[index];
  index += sizeof(packet->data[index]);
  output->ledout = output->ledout << 8;
  output->ledout += packet->data[index];
  index += sizeof(packet->data[index]);

  output->value = packet->data[index];
  index += sizeof(packet->data[index]);
  output->value = output->value << 8;
  output->value += packet->data[index];
  index += sizeof(packet->data[index]);

  output->timmer = packet->data[index];
  index += sizeof(packet->data[index]);
  output->timmer = output->timmer << 8;
  output->timmer += packet->data[index];
  index += sizeof(packet->data[index]);

#ifdef DEBUG
  writeRS();
  mySerial.print("pinout = 0x");
  mySerial.println(output->pinout, HEX);
  mySerial.print("ledout = 0x");
  mySerial.println(output->ledout, HEX);
  mySerial.print("value = 0x");
  mySerial.println(output->value, HEX);
  mySerial.print("timmer = 0x");
  mySerial.println(output->timmer, HEX);
  readRS();
#endif
  return OK;
}

uint8_t parse_dimming_packet(dimming_t *dimming, modbus_rcu *packet)
{
  uint8_t index = 0;
  dimming->channel = packet->data[index];
  index += sizeof(packet->data[index]);
  dimming->channel = dimming->channel << 8;
  dimming->channel += packet->data[index];
  index += sizeof(packet->data[index]);

  dimming->value = packet->data[index];
  index += sizeof(packet->data[index]);
  dimming->value = dimming->value << 8;
  dimming->value += packet->data[index];
  index += sizeof(packet->data[index]);

#ifdef DEBUG
  writeRS();
  mySerial.print("channel = 0x");
  mySerial.println(dimming->channel, HEX);
  mySerial.print("value = 0x");
  mySerial.println(dimming->value, HEX);
  readRS();
#endif
  return OK;
}

uint8_t parse_sensor_packet(sensor_t *sensor, modbus_rcu *packet)
{
  uint8_t index = 0;
  sensor->sensor_pin = packet->data[index];
  index += sizeof(packet->data[index]);
  sensor->sensor_pin = sensor->sensor_pin << 8;
  sensor->sensor_pin += packet->data[index];

#ifdef DEBUG_ADD_SENSOR
  writeRS();
  mySerial.print("parse: add sensor pin = 0x");
  mySerial.println(sensor->sensor_pin, HEX);
  readRS();
#endif
  return OK;
}

uint8_t parse_air_coner_packet(air_condiotioner_t *air_condiotioner, modbus_rcu *packet)
{
  uint8_t index = 0;

  air_condiotioner->channel = packet->data[index];
  index += sizeof(packet->data[index]);
  air_condiotioner->channel = air_condiotioner->channel << 8;
  air_condiotioner->channel += packet->data[index];
  index += sizeof(packet->data[index]);

  air_condiotioner->power = packet->data[index];
  index += sizeof(packet->data[index]);
  air_condiotioner->power = air_condiotioner->power << 8;
  air_condiotioner->power += packet->data[index];
  index += sizeof(packet->data[index]);

  air_condiotioner->fan = packet->data[index];
  index += sizeof(packet->data[index]);
  air_condiotioner->fan = air_condiotioner->fan << 8;
  air_condiotioner->fan += packet->data[index];
  index += sizeof(packet->data[index]);

  air_condiotioner->mode = packet->data[index];
  index += sizeof(packet->data[index]);
  air_condiotioner->mode = air_condiotioner->mode << 8;
  air_condiotioner->mode += packet->data[index];
  index += sizeof(packet->data[index]);

  air_condiotioner->temperature = packet->data[index];
  index += sizeof(packet->data[index]);
  air_condiotioner->temperature = air_condiotioner->temperature << 8;
  air_condiotioner->temperature += packet->data[index];
  index += sizeof(packet->data[index]);

#ifdef DEBUG_AIR_CONDITIONING
  writeRS();
  mySerial.print("channel = 0x");
  mySerial.println(air_condiotioner->channel, HEX);
  mySerial.print("power = 0x");
  mySerial.println(air_condiotioner->power, HEX);
  mySerial.print("fan = 0x");
  mySerial.println(air_condiotioner->fan, HEX);
  mySerial.print("mode = 0x");
  mySerial.println(air_condiotioner->mode, HEX);
  mySerial.print("temperature = 0x");
  mySerial.println(air_condiotioner->temperature, HEX);
  readRS();
#endif
  return OK;
}

void sentEventButtonModbus(uint8_t button)
{

  if (button == 0)
  {
    return;
  }
  packet_event_button.address = my_addr;
  packet_event_button.func = BUTTON;
  packet_event_button.coil = REG_BUTTON;
  packet_event_button.button = button;

  uint8_t index = 0;
  packet_event_button.arr[index] = packet_event_button.address;
  index++;
  packet_event_button.arr[index] = packet_event_button.func;
  index++;
  packet_event_button.arr[index] = (uint8_t)(packet_event_button.coil >> 8);
  index++;
  packet_event_button.arr[index] = packet_event_button.coil & 0xff;
  index++;
  packet_event_button.arr[index] = (uint8_t)(packet_event_button.button >> 8);
  index++;
  packet_event_button.arr[index] = packet_event_button.button & 0xff;
  index++;
  fillCRC((uint8_t *)&packet_event_button.arr, LEN_PACKET_BUTTON);

  writeRS();
  for (uint8_t i = 0; i < LEN_PACKET_BUTTON; i++)
  {
    mySerial.write(packet_event_button.arr[i]);
  }
  readRS();
}

uint16_t calCRC16(uint8_t *buff, int len)
{
  uint16_t i, j, cs;
  cs = INIT_CHECKSUM;
  for (j = 0; j < len; j++)
  {
    cs = cs ^ buff[j];
    for (i = 0; i < 8; i++)
    {
      if (((cs) & 0x0001) == 1)
        cs = (cs >> 1) ^ CRC16_MODBUS;
      else
        cs = cs >> 1;
    }
  }
  return cs;
}

void fillCRC(uint8_t *buff, int len)
{
#ifdef DEBUG_SEND_DATA
  writeRS();
  mySerial.println("before calculator: ");
  uint8_t *index = buff;
  for (int i = 0; i < len; i++)
  {
    mySerial.print(*index, HEX);
    index++;
    mySerial.print(" ");
  }
  mySerial.println();
  readRS();
#endif

  uint16_t crc = calCRC16((uint8_t *)buff, len - sizeof(uint16_t));
  *(buff + len - 2) = LO_UINT16(crc);
  *(buff + len - 1) = HI_UINT16(crc);

#ifdef DEBUG_SEND_DATA
  writeRS();
  mySerial.println("after calculator: ");
  index = buff;
  for (int i = 0; i < len; i++)
  {
    mySerial.print(*index, HEX);
    index++;
    mySerial.print(" ");
  }
  mySerial.println();
  readRS();
#endif
}

#ifdef DEBUG_ADDR
void debugAddr()
{
  writeRS();
  mySerial.print("my_addr = ");
  mySerial.println(my_addr);
  // mySerial.println(getMyAddress());
  readRS();
}
#endif

void writeRS()
{
  digitalWrite(enRS, HIGH);
  delay(1);
}

void readRS()
{
  delay(3);
  digitalWrite(enRS, LOW);
}

void printLog(const String log)
{
  writeRS();
  mySerial.print(log);
  readRS();
}

void printLogln(const String log)
{
  writeRS();
  mySerial.println(log);
  readRS();
}

// handle function for golden imperial
void sentEventDoorSensor(uint8_t status)
{
  uint8_t data_send[LEN_PACKET_BUTTON] = {0};
  data_send[0] = my_addr;
  data_send[1] = BUTTON;
  data_send[2] = 0x00;
  data_send[3] = 0x02;
  data_send[4] = status;
  data_send[5] = BUTTON7;
  fillCRC(data_send, LEN_PACKET_BUTTON);
  writeRS();
  for (uint8_t i = 0; i < LEN_PACKET_BUTTON; i++)
  {
    mySerial.write(data_send[i]);
  }
  readRS();
}

void sentEventMotionSensor(uint8_t status)
{
  uint8_t data_send[LEN_PACKET_BUTTON] = {0};
  data_send[0] = my_addr;
  data_send[1] = BUTTON;
  data_send[2] = 0x00;
  data_send[3] = 0x02;
  data_send[4] = status;
  data_send[5] = BUTTON8;
  fillCRC(data_send, LEN_PACKET_BUTTON);
  writeRS();
  for (uint8_t i = 0; i < LEN_PACKET_BUTTON; i++)
  {
    mySerial.write(data_send[i]);
  }
  readRS();
}