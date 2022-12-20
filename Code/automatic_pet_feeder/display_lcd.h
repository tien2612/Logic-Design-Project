#ifndef display_lcd_h
#define display_lcd_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>

#define LCDADDR 0x27
#define RP_MODE      1                        // State of remaining food in the plate state
#define RC_MODE      2                        // State of remaining food in the container
#define RF_MODE      3                        // State of food released each time
#define MF_MODE      4                        // State of max food per day (user set)

#define T0_MODE      5                        // State of time0 to feed
#define T1_MODE      6                        // State of time1 to feed
#define T2_MODE      7                        // State of time2 to feed

#define INIT_MODE            10
#define SCHEDULE0_MODE       11                       // Set schedule for schedule 0
#define SCHEDULE1_MODE       12                       // Set schedule for schedule 1
#define SCHEDULE2_MODE       13                       // Set schedule for schedule 2
#define SCHEDULE0_ACTIVE     14
#define SCHEDULE1_ACTIVE     15
#define SCHEDULE2_ACTIVE     16

extern double readingCurrFood;
extern double readingRemainFood;
extern double lastReadingCurrFood;
extern double lastReadingRemainFood;

extern LiquidCrystal_I2C lcd;

extern int mode;                     // State to display on LCD (for button0).
extern int counter;                             // Variable for calculating the total number of digits.
extern int status;              
extern int menu;

/* Declare for any flag here. */

extern bool flag_settingSchedule;
extern bool flag_settingMaxFood;
extern bool flag_sch0_active;           // If flag = 1 then feed the animal according to the time0 and vice versa.
extern bool flag_sch1_active;            // If flag = 1 then feed the animal according to the time1 and vice versa.
extern bool flag_sch2_active;            // If flag = 1 then feed the animal according to the time2 and vice versa.
extern bool FIRST_PRESS_RESET;
extern bool flag_confirm;
extern bool flag_confirm_with_keypad;
extern bool waitForPet;

struct foodReleased {
  char food[3] = {'3', '0', '0'};
  int index;
};

struct indexOfCharKeypad {
  int releasedFood[4];
  int flagReleasedFood[4];
  int flagmaxFood;
  int maxFood;
};

extern char MAX_FOOD_PER_DAY_array[4];
extern char setSchedule0[6];
extern char setSchedule1[6];
extern char setSchedule2[6];

extern indexOfCharKeypad indexKeypad;
extern foodReleased foodReleasedEachTime_array[4];

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