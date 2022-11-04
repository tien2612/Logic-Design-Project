// AUTOMATIC PET FEEDER
// HOW TO USE?
// Button0: Press once to switch between food remains and food in container and time to feed information.
// Button1: Setup time to feed your pet. Hold the button to switch between states (hour->minute->second->hour1->,..->hour->minute->,..). It depends on your current state.
// Button2: Set the maximum amount of food to feed per day (it can only be set to 1600g). Press once to increase 200 and hold the button to decrease the value by 200.
// Button3: Set the food is released each time (it can only be set to 1600g). Press once to increase 10 and hold the button to decrease the value by 10.
// Button4: Reset all settings. If you press once, a notification will be shown to confirm; if you press again, settings will be reset.
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include "Key.h"
#include <Keypad_I2C.h>
#include <Keypad.h>        // GDY120705
#include <Wire.h>

#define IS_PRESSED  HIGH
#define RP_MODE      1                        // State of remaining food in the plate state
#define RC_MODE      2                        // State of remaining food in the container
#define RF_MODE      3                        // State of food released each time
#define MF_MODE      4                        // State of max food per day (user set)

#define T0_MODE      5                        // State of time0 to feed
#define T1_MODE      6                        // State of time1 to feed
#define T2_MODE      7                        // State of time2 to feed

#define INIT_MODE            10
#define SCHEDULE0_MODE       11                       // Set schedule for hour0
#define SCHEDULE1_MODE       12                       // Set schedule for minute0
#define SCHEDULE2_MODE       13                       // Set schedule for second0

#define TIME_NOTHING_CHANGES  20000           // If nothing changes after 20S, then turn off LCD to save power
#define MAX_FOOD              1600                     // Per day food allowance.
#define MIN_FOOD              0

#define INFOR_MENU                30
#define SET_SCHEDULE_MENU         31
#define SET_RELEASED_FOOD_MENU    32
#define SET_MAX_FOOD_MENU         33
#define RESET_MENU                34

// TOUCH
#define TOUCH_SENSOR 41
#define TOUCH_OUT    37

#define I2CADDR 0x21

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
	};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad
	
Keypad_I2C keypad = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);

// Declare for any var about time here.
unsigned long startMillis;    // Some global variables available anywhere in the program
unsigned long currentMillis; // Current time since the program started 
char setSchedule0[6], setSchedule1[6], setSchedule2[6] = {0};

// Declare for any flag here.
bool flag_longclick_btn1;
bool flag_settingSchedule;
bool flag_duringLongPress;
bool flag_settingMaxFood;
bool flag_sch0_active;         // If flag = 1 then feed the animal according to the time0 and vice versa.
bool flag_sch1_active;         // If flag = 1 then feed the animal according to the time1 and vice versa.
bool flag_sch2_active;         // If flag = 1 then feed the animal according to the time2 and vice versa.
bool FIRST_PRESS_BUTTON0;
bool FIRST_PRESS_BUTTON1;
bool FIRST_PRESS_BUTTON2;
bool FIRST_PRESS_BUTTON3;
bool FIRST_PRESS_RESET;
bool flag_confirm = false;
// Declare for the button or something else. 

int btn[4] = {7, 10, 11, 12};
int mode = RC_MODE;                      // State to display on LCD (for button0).
int counter;                   // Variable for calculating the total number of digits.
int status = SCHEDULE0_MODE;                   
int menu = 1;
// Declare about food here
int remaining_food = 0;
int MAX_FOOD_PER_DAY = 1000;
int foodReleasedEachTime = 150;
int remainingFoodInContainer;

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">All Information");
      lcd.setCursor(0, 1);
      lcd.print("Set Schedule");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("All Information");
      lcd.setCursor(0, 1);
      lcd.print(">Set Schedule");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">Food is released");
      lcd.setCursor(0, 1);
      lcd.print("Set max food");
      break;
    case 4:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Food is released");
      lcd.setCursor(0, 1);
      lcd.print(">Set max food");
      break;
    case 5:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">Reset settings");
      break;
    case 6:
      menu = 5;
      break;
  }
}
int calDigitofNumber(int number) {
    int count = 0;
    int temp = number;
    do {
      temp /= 10;
      ++count;
    } while (temp != 0);
    return count;
}
// Display the remaining food on the LCD. 
void displayRemainingFood_LCD(int status) {
    lcd.init();                      // Initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(1,0);
    lcd.print("REMAINING FOOD");

    // Display (g) is still on the plate. 
    int count = calDigitofNumber(remaining_food);
    switch(status) {
      case RP_MODE:
          lcd.setCursor(1, 1);
          lcd.print("PLATE:");
          lcd.setCursor(8, 1);
          lcd.print(remaining_food);
          lcd.setCursor(8 + count + 1, 1);
          lcd.print("G");
          break;
      case RC_MODE:
          lcd.setCursor(1, 1);
          lcd.print("CONTAINER:");
          lcd.setCursor(11, 1);
          lcd.print(remainingFoodInContainer);
          lcd.setCursor(11 + count, 1);
          lcd.print("G");
          break;
    }
}
// Display feed time on LCD 
void displayTimeSchedule_LCD(char setSchedule[6]) {
    int current_cursor = 0;
    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(0,0);
    if (flag_settingSchedule) {
        lcd.setCursor(1,0);
        switch(status) {
            case SCHEDULE0_MODE:
                 lcd.print("SET SCHEDULE0");
                 break;
            case SCHEDULE1_MODE:
                 lcd.print("SET SCHEDULE1");
                 break;
            case SCHEDULE2_MODE:
                 lcd.print("SET SCHEDULE2");
                 break;   
            default:
                 lcd.print("TIME TO FEED");
                 break; 
        }  
    } else {
          lcd.print("TIME");
          lcd.setCursor(4, 0);
          switch(mode) {
            case T0_MODE:
                lcd.print(0);
                lcd.setCursor(5, 0);
                if (flag_sch0_active) {
                    lcd.print(" - ACTIVE");
                } else lcd.print(" - INACTIVE");
                break;
            case T1_MODE:
                lcd.print(1);
                if (flag_sch1_active) {
                    lcd.print(" - ACTIVE");
                } else lcd.print(" - INACTIVE");
                break;
            case T2_MODE:
                lcd.print(2);
                if (flag_sch2_active) {
                    lcd.print(" - ACTIVE");
                } else lcd.print(" - INACTIVE");
                break;
            default:
              break;              
          }
    }
    // Display hour
    lcd.setCursor(4, 1);       
    lcd.print(setSchedule[0]);
    lcd.setCursor(5, 1);    
    lcd.print(setSchedule[1]);
    lcd.setCursor(6, 1); 
    lcd.print(":");
    // Display minute
    lcd.setCursor(7, 1);       
    lcd.print(setSchedule[2]);
    lcd.setCursor(8, 1);    
    lcd.print(setSchedule[3]);
    lcd.setCursor(9, 1); 
    lcd.print(":");
    // Display second
    lcd.setCursor(10, 1);       
    lcd.print(setSchedule[4]);
    lcd.setCursor(11, 1);    
    lcd.print(setSchedule[5]);
}
// Set max food per day
void displayMaxFood() {
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("MAX FOOD PER DAY");
    lcd.setCursor(6,1);
    lcd.print(MAX_FOOD_PER_DAY);
}

void displayFoodReleased() {
    int count = calDigitofNumber(foodReleasedEachTime);
    lcd.init();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FOOD IS RELEASED");
    lcd.setCursor(0,1);
    lcd.print("EACH TIME:");
    lcd.setCursor(11, 1);
    lcd.print(foodReleasedEachTime);
    lcd.setCursor(11 + count + 1, 1);
    lcd.print("G");
}

int index_schedule_keypad = 0;

void setup()
{
    lcd.home();
    lcd.init();
    Serial.begin(9600);
    keypad.begin(makeKeymap(keys));
    Wire.begin();
    pinMode(btn[0], INPUT_PULLUP);
    pinMode(btn[1], INPUT_PULLUP);
    pinMode(btn[2], INPUT_PULLUP);
    pinMode(btn[3], INPUT_PULLUP);

    flag_sch0_active = 0;
    flag_sch1_active = 0;
    flag_sch2_active = 0;
    FIRST_PRESS_RESET = 1; 
    startMillis = millis();                          // Initial start time.
    updateMenu();
}

void loop()
{
  for (int k = 0; k < 6; k++) Serial.print(setSchedule0[k]);
  currentMillis = millis();  // Get the current "time" (actually the number of milliseconds since the program started)
  // Turn off LCD after 15s if nothing changes
  if (currentMillis - startMillis >= TIME_NOTHING_CHANGES) {
      lcd.noBacklight();
      lcd.noDisplay();
  }
  if ((digitalRead(btn[0]) || digitalRead(btn[1]) || digitalRead(btn[2]) || digitalRead(btn[3])) == IS_PRESSED) startMillis = millis();
  // Switching text if button 0 is pressed
  if (!digitalRead(btn[2])){
    flag_confirm = false;
    if (menu >= 5) menu = 1;
    else menu++;
    updateMenu();
    delay(10);
    while (!digitalRead(btn[2]));
  }

  if (!digitalRead(btn[3])){
    flag_confirm = false;
    if (menu <= 1) menu = 5;
    else menu--;
    updateMenu();
    delay(10);
    while(!digitalRead(btn[3]));
  }

  if (!digitalRead(btn[0])){
    flag_confirm = true;
    
    if (flag_settingSchedule) {
      
      index_schedule_keypad = 0;
      switch(status) {
        case SCHEDULE0_MODE:
            status = SCHEDULE1_MODE;
            displayTimeSchedule_LCD(setSchedule1);
            break;
        case SCHEDULE1_MODE:
            status = SCHEDULE2_MODE;
            displayTimeSchedule_LCD(setSchedule2);
            break;
        default:
            break;
      }
    }
    delay(10);
    while (!digitalRead(btn[0]));
  }

  if (flag_confirm) executeAction(); // Confirm action.
}

//*** Functions for button4 ***//

void printConfirm() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("PRESS AGAIN");
  lcd.setCursor(2, 1);
  lcd.print("TO CONFIRM");
}
// Func to set all elements of an array is equal to 0
void setZero(char arr[6]) {
  for (int i = 0; i < 3; i++) {
    arr[i] = 0;
  }
}
void clearSettings() {
    if (FIRST_PRESS_RESET) {
        printConfirm();
        FIRST_PRESS_RESET = 0;
    } 
    else {
        MAX_FOOD_PER_DAY = 0;
        foodReleasedEachTime = 0;
        setZero(setSchedule0); setZero(setSchedule1); setZero(setSchedule2);
        lcd.init();
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("DONE !!");
        FIRST_PRESS_RESET = 1;
    } 
}

void executeAction() {
  switch (menu) {
    case 1:
        switch(mode) {
            case RC_MODE: 
                displayRemainingFood_LCD(RC_MODE);
                mode = RP_MODE;
                break;
            case RP_MODE: 
                displayRemainingFood_LCD(RP_MODE);
                mode = RF_MODE;
                break;
            case RF_MODE:
                displayFoodReleased();
                mode = T0_MODE;
                break;
            case T0_MODE:
                displayTimeSchedule_LCD(setSchedule0);
                mode = T1_MODE;
                break;
            case T1_MODE:
                displayTimeSchedule_LCD(setSchedule1);
                mode = T2_MODE;
                break;
            case T2_MODE:
                displayTimeSchedule_LCD(setSchedule2);
                mode = RC_MODE;
                break;
            default:
                lcd.clear();
      }
      break;
    case 2:
      flag_settingSchedule = 1;
      char key = keypad.getKey();// Read the key
      if (key){
          if (index_schedule_keypad >= 6) index_schedule_keypad = 0;
          switch(status) {
              case SCHEDULE0_MODE:
                    setSchedule0[index_schedule_keypad] = key;
                    index_schedule_keypad++;
                    displayTimeSchedule_LCD(setSchedule0);
                    break;
              case SCHEDULE1_MODE:
                    setSchedule1[index_schedule_keypad] = key;
                    index_schedule_keypad++;
                    displayTimeSchedule_LCD(setSchedule1);
                    break;
              case SCHEDULE2_MODE:
                    setSchedule2[index_schedule_keypad] = key;
                    index_schedule_keypad++;
                    displayTimeSchedule_LCD(setSchedule2);
                    break;   
              default:
                    lcd.print("TIME TO FEED");
                    break; 
          }  
      } 
      //action2();
      break;
    case 3:
     // action3();
      break;
    case 4:
     // action4();
      break;
    case 5:
    //  action4();
      break;
  }
}

void action1() {
  lcd.clear();
  lcd.print(">Executing #1");
  delay(1500);
}