// AUTOMATIC PET FEEDER
// HOW TO USE?
// Button0: Press once to switch between food remains and time to feed information.
// Button1: Setup time to feed your pet.
// Button2: Set the maximum amount of food to feed per day (it can only be set to 1600g). 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>

#define IS_PRESSED  HIGH
#define R_MODE      1     // display remaining food on LCD
#define T_MODE      2     // display time to feed on LCD
#define INIT_MODE   4
#define H_MODE      5     // Set schedule for hour
#define M_MODE      6     // Set schedule for minute
#define S_MODE      7     // Set schedule for second
#define TIME_NOTHING_CHANGES  10000
OneButton btn1(51, true);
unsigned long startMillis;  // Some global variables available anywhere in the program
unsigned long currentMillis; // Current time since the program started 

int MAX_FOOD_PER_DAY;
int remaining_food;
int btn[3] = {53, 51, 49};
int hour, minute, second;
int mode = R_MODE;
int counter;
int status;
extern volatile unsigned long timer0_millis;
int flag_longclick_btn1;
int flag_settingSchedule;

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// Display the remaining food on the LCD. 
void displayRemainingFood_LCD() {
    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(1,0);
    lcd.print("REMAINING FOOD");

    // Display kg
    int count = 0;
    int temp = remaining_food;
    do {
      temp /= 10;
      ++count;
    } while (temp != 0);
    int current_cursor = (16 - count - 3) / 2;
    lcd.setCursor(current_cursor, 1);
    lcd.print(remaining_food);
    lcd.setCursor(current_cursor + count + 1, 1);
    lcd.print("G");
}
// Display feed time on LCD 
void displayTimeSchedule_LCD(int hour, int minute, int second) {
    int current_cursor = 0;
    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(2,0);
    if (flag_settingSchedule) {
        switch(status) {
            case H_MODE:
                 lcd.print("SET HOUR");
                 break;
            case M_MODE:
                 lcd.print("SET MINUTE");
                 break;
            case S_MODE:
                 lcd.print("SET SECOND");
                 break;  
            default:
                 lcd.print("TIME TO FEED");
                 break; 
        }  
    }
    else lcd.print("TIME TO FEED");
    // Display hour
    current_cursor = 5;
    lcd.setCursor(current_cursor, 1);       
    lcd.print(hour);
    if (hour < 10) current_cursor++;
    else current_cursor = current_cursor + 2;
    lcd.setCursor(current_cursor, 1);
    lcd.print(":");
    // Display minute
    if (minute < 10) {
        lcd.setCursor(++current_cursor, 1);
        lcd.print(0);
    }
    lcd.setCursor(++current_cursor, 1);
    lcd.print(minute);
    lcd.setCursor(++current_cursor, 1);
    lcd.print(":");
    // Display second
    if (second < 10) {
        lcd.setCursor(++current_cursor, 1);
        lcd.print(0);
    }
    lcd.setCursor(++current_cursor, 1);
    lcd.print(second);
}
// Set max food per day
void setMaxFood() {
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("MAX FOOD PER DAY");
  lcd.setCursor(6,1);
  MAX_FOOD_PER_DAY += 200;
  lcd.print(MAX_FOOD_PER_DAY);
}
// Check if the button is holding more than 1s
bool isHoldingMoreThan1s(int button) {
  while (digitalRead(button) == HIGH) {
    counter++;
    if (counter >= 1000) {
      counter = 0;
      return true;
    }
  }
  return false;
}
// Don't allow the user to set more food per day (reach max).
void Exceeding() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("CAN'T SET MORE");
}

void setup()
{
    Serial.begin(115200);
    pinMode(btn[0], INPUT);
    pinMode(btn[1], INPUT);
    remaining_food = 0;
    MAX_FOOD_PER_DAY = 0;
    hour = 0, minute = 0, second = 0;
    flag_longclick_btn1 = 0;
    flag_settingSchedule = 0;
    startMillis = millis();  // Initial start time
    status = INIT_MODE;
}

void loop()
{
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  // Turn off LCD after 15s if nothing changes
  if (currentMillis - startMillis >= TIME_NOTHING_CHANGES) {
      lcd.noBacklight();
      lcd.noDisplay();
  }
  if ((digitalRead(btn[0]) || digitalRead(btn[1]) || digitalRead(btn[2])) == IS_PRESSED) startMillis = millis();
  // Switching text if button 0 is pressed
  if (digitalRead(btn[0]) == IS_PRESSED) {
    flag_settingSchedule = 0;
    switch(mode) {
    case R_MODE: 
        displayRemainingFood_LCD();
        mode = T_MODE;
        break;  
    case T_MODE: 
        displayTimeSchedule_LCD(hour, second, minute);
        mode = R_MODE;
        break;
    default:
        lcd.clear();
    }
  }
  // Setup schedule with button 2
  // If you hold the button for at least 1 seconds, then set for an hour/minute/second depending on the current state.
  if (isHoldingMoreThan1s) {
    switch (status) {
      case INIT_MODE:
          status = H_MODE;
          break;
      case H_MODE:
          status = M_MODE;
          break;
      case M_MODE:
          status = S_MODE;
          break;
      case S_MODE:
          status = H_MODE;
          break;
      default:
          status = 0;     
    }
  }
  if (digitalRead(btn[1]) == IS_PRESSED) {
          flag_settingSchedule = 1;
            switch(status) {
              case H_MODE:
                   if (digitalRead(btn[1]) == IS_PRESSED) {
                       displayTimeSchedule_LCD(hour, minute, second);
                       if (hour >= 24) hour = 0;
                       else hour++;
                   } 
                   break;
              case M_MODE:
                   if (digitalRead(btn[1]) == IS_PRESSED) {
                       displayTimeSchedule_LCD(hour, minute, second);
                       if (minute >= 60) minute = 0;
                       else minute++;
                   }
                   break;
              case S_MODE:
                  if (digitalRead(btn[1]) == IS_PRESSED) {
                       displayTimeSchedule_LCD(hour, minute, second);
                       if (second >= 60) second = 0;
                       else second++;
                  }
                  status = H_MODE;
                  break;
          }
  }
  // Setup max food per day, with a max initial value of 0. To increase the 200, press once.
  if (digitalRead(btn[2]) == IS_PRESSED) {
      flag_settingSchedule = 0;
      if (MAX_FOOD_PER_DAY < 1600) setMaxFood();
      else Exceeding();
  }
}
