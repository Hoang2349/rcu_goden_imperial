#include "dimming.h"
#include "MCP492X.h"

MCP492X myDac;

void setupDimming()
{
  myDac.begin();
}

void controlDimming(uint8_t channel, uint8_t level)
{
  uint16_t levelTmp = map(level, LEVEL_INPUT_MIN, LEVEL_INPUT_MAX, LEVEL_RAW_MIN, LEVEL_RAW_MAX);
  myDac.analogWrite( channel, levelTmp);
}
