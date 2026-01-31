#ifndef DIMMING_H
#define DIMMING_H

#define LEVEL_RAW_MAX 4095
#define LEVEL_RAW_MIN 0
#define LEVEL_INPUT_MAX 100
#define LEVEL_INPUT_MIN 0

#include <arduino.h>

void setupDimming();
void controlDimming(uint8_t channel, uint8_t level);

#endif
