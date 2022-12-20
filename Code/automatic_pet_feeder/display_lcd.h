#ifndef display_lcd_h
#define display_lcd_h

#include "Arduino.h"

int calDigitofNumber(int number);
// Display the remaining food on the LCD. 
void displayRemainingFood_LCD(int status);
// Display feed time on LCD 
void displayTimeSchedule_LCD(char setSchedule[6]);
// Set max food per day
void displayMaxFood();

void displayFoodReleased(int index);

void setActiveSchedule(int num, bool flag_sch_active);

#endif