/*
   MCP4921/MCP4922 12 bit DAC Library
   By MichD

   GitHub: https://github.com/michd/Arduino-MCP492X

   The Microchip MCP4921 and MCP4922 are 12 bit digital to analog converters
   with an SPI interface. The MCP4921 has one DAC output, the MCP4922 has two.

   This library is built around the Arduino default SPI library and uses the
   designated SPI pins, in addition to a configurable chip select pin, passed
   in the constructor.
   Look up the designated pins for your arduino board, and wire up as follows:

*/

#include <SPI.h>
#include "MCP492X.h"
#include <Arduino.h>

#define TMP_MISO
#ifdef TMP_MISO
#define TMP_MISO_SPI PB4
#endif


SPIClass SPI_1(DAC_SDI, TMP_MISO_SPI, DAC_CLK);

MCP492X::MCP492X() {
}

void MCP492X::begin() {

  //SPI_1.setMOSI(_pinSDI);
  // Temporary set MISO = PB4
  //SPI_1.setMISO(TMP_MISO_SPI);
  //SPI_1.setSCLK(_pinCLK);
//HieuNV - Day la gi?
  pinMode(PA5, OUTPUT);
  digitalWrite(PA5, HIGH);
  // pinMode(_pinCLK, OUTPUT);
  // digitalWrite(_pinCLK, LOW);
  //pinMode(_pinSDI, OUTPUT);

  SPI_1.begin(DAC_CS);
  pinMode(DAC_CS, OUTPUT);
  digitalWrite(DAC_CS, HIGH);
  //SPI_1.setClockDivider(SPI_CLOCK_DIV16);

  //pinMode(TMP_MISO_SPI, INPUT);
  //digitalWrite(TMP_MISO_SPI, LOW);
  SPI_1.setBitOrder(MSBFIRST);          		//  MSB to be sent first
  SPI_1.setDataMode(SPI_MODE0);         		//  Set for clock rising edge, clock idles low
  //SPI_1.setClockDivider(SPI_CLOCK_DIV16  );		//  Set clock divider (optional)
  //	delay(100);
  _spiSettings = SPISettings(14000000, MSBFIRST, SPI_MODE0, SPI_TRANSMITONLY );
  // SPI_1.beginTransaction(DAC_CS, _spiSettings);

}

void MCP492X::analogWrite(unsigned int value) {
  analogWrite(0, value);
}

// Only applies to MCP4922
void MCP492X::analogWrite(bool odd, unsigned int value) {
  analogWrite(
    odd, // Pass channel
    1,   // Buffered,
    1,   // Gain mode 1x (0 = 2x)
    1,   // Don't shut down output
    value);
}

// If you want full control, this method lets you set all config bits
// See MCP492X datasheet page 18 ("5.0 Serial interface") for details
void MCP492X::analogWrite(
  bool odd, bool buffered, bool gain, bool active, unsigned int value) {

  uint8_t  configBits = odd << 3 | buffered << 2 | gain << 1 | active;

  // Compose the first byte to send to the DAC:
  // the 4 control bits, and the 4 most significant bits of the value
  uint8_t  firstByte = (configBits << 4 | ( value & 0xF00) >> 8) & 0xFF;
  // Second byte is the lower 8 bits of the value
  uint8_t  secondByte = value & 0xFF;

  //uint8_t data[] = {firstByte, secondByte};
  _beginTransmission();
  //digitalWrite(DAC_CS, 0);
  //SPI_1.transfer(DAC_CS, data, 2);
  //SPI_1.transfer(DAC_CS,  firstByte , SPI_CONTINUE);
  //SPDR = firstByte;
  //delay(10);
  //digitalWrite(_pinSDI, HIGH);
  //SPDR = secondByte;
  //SPI_1.transfer(DAC_CS, secondByte, SPI_LAST      );
  SPI_1.transfer(DAC_CS,  firstByte , SPI_CONTINUE);

  SPI_1.transfer(DAC_CS, secondByte, SPI_LAST      );

  //digitalWrite(DAC_CS, 1);
  digitalWrite(PA5, LOW);
  //digitalWrite(DAC_SDI, HIGH);
  //delay(1);
  digitalWrite(PA5, HIGH);
  //delay(10);
  _endTransmission();
}

void MCP492X::_beginTransmission() {
  digitalWrite(DAC_CS, 0);
  //SPI_1.beginTransaction(DAC_CS, _spiSettings);
}

void MCP492X::_endTransmission() {
  //SPI_1.endTransaction(DAC_CS);
  digitalWrite(DAC_CS, 1);
}
