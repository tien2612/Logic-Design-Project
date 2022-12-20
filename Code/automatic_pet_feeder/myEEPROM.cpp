#include "myEEPROM.h"


void eepromRead(uint16_t addr, void* output, uint16_t length) {
  uint8_t* src; 
  uint8_t* dst;
  src = (uint8_t*)addr;
  dst = (uint8_t*)output;
  for (uint16_t i = 0; i < length; i++) {
      *dst++ = eeprom_read_byte(src++);
  }
}

void eepromWrite(uint16_t addr, void* input, uint16_t length) {
    uint8_t* src; 
    uint8_t* dst;
    src = (uint8_t*)input;
    dst = (uint8_t*)addr;
    for (uint16_t i = 0; i < length; i++) {
        eeprom_write_byte(dst++, *src++);
    }
}

void eepromWriteChar(int address, char* numbers, int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    EEPROM.write(addressIndex, numbers[i]);
    //EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);
    addressIndex += 1;
  }
}
void eepromReadChar(int address, char* numbers, int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = (EEPROM.read(addressIndex));
    addressIndex += 1;
  }
}