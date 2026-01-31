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

#include <Arduino.h>
#include <SPI.h>

// Ensure we don't double-define the functionality
#ifndef MCP492X_h
#define MCP492X_h
#define STM32G030K8T6
#ifdef STM32G030K8T6
#define DAC_SDI PB5
//#define DAC_LATCH PA15
#define DAC_CLK PB3
#define DAC_CS PA15
#endif



class MCP492X {
  public:
    // Constructor, takes the chip select pin
    // Use outside any functions:
    // `MCP492X myDac(pinNumber);`
    MCP492X();

    // Initilize, starts the SPI bus. Call in setup()
    // Example:
    // ```
    // void setup() {
    //   myDac.begin();
    // }
    //
    void begin();

    // Writes a 12 bit value to the output.
    // If on the MCP4922, defaults to DAC output 0 (A).
    // Example:
    // ```
    // myDac.analogWrite(1234);
    // ```
    void analogWrite(unsigned int);       // Write a 12 bit value

    // Writes a 12 bit value to a given DAC output (0 or 1 / A or B)
    // Param 1 = DAC selection
    // Param 2 = 12 bit value
    // Example:
    // ```
    // myDac.analogWrite(1, 4095);
    // ```
    void analogWrite(bool, unsigned int); // Write a 12 bit value to a specific output (only MCP4922)

    // Writes a 12 bit value to a given DAC output (0 or 1 / A or B),
    // and allows setting every config bit individually.
    // Param 1 = DAC selection
    // Param 2 = buffer input
    // Param 2 = gain mode (1 = 1x, 0 = 2x)
    // Param 3 = shut down output (1 = active, 0 = shutdown)
    // Param 4 = 12 bit value
    // Example:
    // ```
    // myDac.analogWrite(1, 0, 1, 1, 2480);
    // ```
    void analogWrite(bool, bool, bool, bool, unsigned int); // Full control over control bits

  private:
    // Internal fields/methods you should not need to worry about.
    // Holds onto the chip select pin number
    // SPI settings for this chip, set up in begin()
    SPISettings _spiSettings;

    // Internal helpers to start/end transmission
    void _beginTransmission();
    void _endTransmission();
};

#endif // MCP921X_h
