#ifndef myEEPROM_H
#define myEEPROM_H

#include "Arduino.h"
#include <EEPROM.h>


void eepromRead(uint16_t addr, void* output, uint16_t length);

void eepromWrite(uint16_t addr, void* input, uint16_t length);

void eepromWriteChar(int address, char* numbers, int arraySize);

void eepromReadChar(int address, char* numbers, int arraySize);
#endif