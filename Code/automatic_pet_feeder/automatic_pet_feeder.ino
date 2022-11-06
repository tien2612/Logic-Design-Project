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

#define I2CADDR 0x21
#define LCDADDR 0x27

#define IS_PRESSED  HIGH
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

#define TIME_NOTHING_CHANGES  30000           // If nothing changes after 20S, then turn off LCD to save power
#define MAX_FOOD              1600                     // Per day food allowance.
#define MIN_FOOD              0

// TOUCH
#define TOUCH_SENSOR A0

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
	};

// I2C
// byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
// byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

byte rowPins[ROWS] = {6, 7, 8, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the keypad

//Keypad_I2C keypad = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Declare for any var about time here.
unsigned long startMillis;    // Some global variables available anywhere in the program
unsigned long currentMillis; // Current time since the program started 
unsigned long startMillisTouch;

// Declare for any flag here.
bool flag_settingSchedule;
bool flag_settingMaxFood;
bool flag_sch0_active;         // If flag = 1 then feed the animal according to the time0 and vice versa.
bool flag_sch1_active;         // If flag = 1 then feed the animal according to the time1 and vice versa.
bool flag_sch2_active;         // If flag = 1 then feed the animal according to the time2 and vice versa.
bool FIRST_PRESS_RESET;
bool flag_confirm = false;
bool flag_confirm_with_keypad = false;
int index_schedule_keypad = 0;
int index_foodisreleased = 0;
int index_maxfood = 0;
// Declare for the button or something else. 

int btn[4] = {10, 7, 11, 12};
int mode = RC_MODE;                      // State to display on LCD (for button0).
int counter;                             // Variable for calculating the total number of digits.
int status = SCHEDULE0_MODE;                   
int menu = 1;
// Declare about food here
int remaining_food = 0;
int MAX_FOOD_PER_DAY = 1000;
int foodReleasedEachTime = 150;
int remainingFoodInContainer;
char foodReleasedEachTime_array[3] = {'1', '5', '0'};
char MAX_FOOD_PER_DAY_array[4] = {'1', '0', '0', '0'};
char setSchedule0[6] = {'0', '0', '0', '0', '0', '0'};
char setSchedule1[6] = {'0', '0', '0', '0', '0', '0'};
char setSchedule2[6] = {'0', '0', '0', '0', '0', '0'};

LiquidCrystal_I2C lcd(LCDADDR,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAX FOOD PER DAY");
    lcd.setCursor(6,1);
    int current_cursor = 6;
    int temp = 6;
    for (int i = 0; i < sizeof(MAX_FOOD_PER_DAY_array)/sizeof(MAX_FOOD_PER_DAY_array[0]); i++ ) {
        if (MAX_FOOD_PER_DAY_array[i] > '9' || MAX_FOOD_PER_DAY_array[i] < '0') break;
        lcd.print(MAX_FOOD_PER_DAY_array[i]);
        temp = i;
    }
    lcd.setCursor(current_cursor + temp + 2, 1);
    lcd.print("G");

}

void displayFoodReleased() {
    lcd.init();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FOOD IS RELEASED");
    lcd.setCursor(0,1);
    lcd.print("EACH TIME:");
    lcd.setCursor(11, 1);
    int current_cursor = 11;
    int temp = current_cursor;
    for (int i = 0; i < sizeof(foodReleasedEachTime_array)/sizeof(foodReleasedEachTime_array[0]); i++ ) {
        if (foodReleasedEachTime_array[i] > '9' || foodReleasedEachTime_array[i] < '0') break;
        lcd.print(foodReleasedEachTime_array[i]);
        temp = i;
    }
    lcd.setCursor(current_cursor + temp + 2, 1);
    lcd.print("G");
}

void setActiveSchedule(int num, bool flag_sch_active) {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("SCHEDULE");
  lcd.setCursor(12, 0);
  lcd.print(num);

  if (flag_sch_active) {
    lcd.setCursor(5, 1);
    lcd.print("ACTIVE");
  } else {
    lcd.setCursor(4, 1);
    lcd.print("INACTIVE");
  }
}

void checkValidSchedule(char *sch_arr) {
  if (sch_arr[0] >= '2') sch_arr[0] = '2';

  if (sch_arr[0] == '2' && sch_arr[1] >= '5') sch_arr[1] = '4';

  if (sch_arr[2] >= '6') sch_arr[2] = '5';

  if (sch_arr[4] >= '6') sch_arr[4] = '5';

}
void setup()
{
    lcd.home();
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    //keypad.begin(makeKeymap(keys));
    Wire.begin();
    pinMode(btn[0], INPUT_PULLUP);
    pinMode(btn[1], INPUT_PULLUP);
    pinMode(btn[2], INPUT_PULLUP);
    pinMode(btn[3], INPUT_PULLUP);

    flag_sch0_active = 0;
    flag_sch1_active = 0;
    flag_sch2_active = 0;
    FIRST_PRESS_RESET = 1; 
    startMillis = millis();   
    startMillisTouch = millis();                       // Initial start time.
    updateMenu();
}


void loop()
{
  char key = customKeypad.getKey();// Read the key
  //Serial.print(key);
  flag_confirm = false;
  //Serial.print(menu);
  currentMillis = millis();  // Get the current "time" (actually the number of milliseconds since the program started)
  // Turn off LCD after 15s if nothing changes
  if ( analogRead(TOUCH_SENSOR) > 650 && (currentMillis - startMillisTouch >= 80) ) {
    Serial.print("touch!!");
    startMillisTouch = millis();
  }
  if (currentMillis - startMillis >= TIME_NOTHING_CHANGES) {
      lcd.noBacklight();
      lcd.noDisplay();
  } else {
    lcd.backlight();
    lcd.display();
  }
  if ((!digitalRead(btn[0]) || !digitalRead(btn[1]) || !digitalRead(btn[2]) || !digitalRead(btn[3]) 
                            || flag_confirm || flag_confirm_with_keypad) ) startMillis = millis();
  // Switching text if button 0 is pressed
  if (!digitalRead(btn[2])){
    flag_confirm = false;
    flag_confirm_with_keypad = false;
    if (menu >= 5) menu = 1;
    else menu++;
    updateMenu();
   // delay(100);
    while (!digitalRead(btn[2])) delay(100);;
  }

  if (!digitalRead(btn[3])){
    flag_confirm = false;
    flag_confirm_with_keypad = false;
    if (menu <= 1) menu = 5;
    else menu--;
    updateMenu();
    
    while(!digitalRead(btn[3])) delay(100);;
  }

  if (!digitalRead(btn[0])){
      flag_confirm = true;
      index_schedule_keypad = 0;
      flag_settingSchedule = false;
      if (menu == 2) {
        flag_settingSchedule = true;
        flag_confirm_with_keypad = true;
          switch(status) {
          case SCHEDULE0_MODE:
                displayTimeSchedule_LCD(setSchedule0);
                break;
          case SCHEDULE0_ACTIVE:
                setActiveSchedule(0, flag_sch0_active);
                break;
          case SCHEDULE1_MODE:
                displayTimeSchedule_LCD(setSchedule1);
                break;
          case SCHEDULE1_ACTIVE:
                setActiveSchedule(1, flag_sch1_active);
          case SCHEDULE2_MODE:
                displayTimeSchedule_LCD(setSchedule2);
                break;
          case SCHEDULE2_ACTIVE:
                setActiveSchedule(2, flag_sch2_active);
                break;
          default:
              break; 
          }
      } else if (menu == 3) {
        flag_settingSchedule = false;
        flag_confirm_with_keypad = true;
        displayFoodReleased();
      } else if (menu == 4) {
          flag_settingSchedule = false;
          flag_confirm_with_keypad = true;
          displayMaxFood();
      } else if (menu == 5) {
          flag_settingSchedule = false;
          flag_confirm_with_keypad = false;
          printConfirm();       
      }
        delay(100);
        while (!digitalRead(btn[0]));
  }

  if (flag_confirm) {
    executeAction(); // Confirm action.
  }

  if (flag_confirm_with_keypad) {
    executeAction(); // Confirm action.
  }
}


void printConfirm() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("PRESS AGAIN");
  lcd.setCursor(2, 1);
  lcd.print("TO CONFIRM");
  //FIRST_PRESS_RESET = 0;
}
// Func to set all elements of an array is equal to 0
void setZero(char *arr) {
    for (size_t i = 0 ; i < strlen (arr); i++) 
      arr[i] = '0';
}

void clearSettings() {
    if (FIRST_PRESS_RESET) {
        FIRST_PRESS_RESET = 0;
    } 
    else {
        setZero(setSchedule0); setZero(setSchedule1); setZero(setSchedule2);
        setZero(foodReleasedEachTime_array); setZero(MAX_FOOD_PER_DAY_array); 
        flag_sch0_active = 0, flag_sch1_active = 0, flag_sch2_active = 0;
        lcd.init();
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("DONE !!");
        FIRST_PRESS_RESET = 1;
    } 
      for (int i = 0; i < 6; i++) Serial.println(setSchedule0[i]);

}

void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
    case 4:
      action4();
      break;
    case 5:
      action5();
      break;
    default:
      break;
  }
}

void action1() {
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
}
void action2() {
  flag_settingSchedule = 1;
  char key = customKeypad.getKey();// Read the key
  //Serial.print(key);
  if (key){
      if (key == 'B') {
        switch(status) {
            case SCHEDULE0_ACTIVE:
              flag_sch0_active = !flag_sch0_active;
              break;
            case SCHEDULE1_ACTIVE:
              flag_sch1_active = !flag_sch1_active;
              break;
            case SCHEDULE2_ACTIVE:
              flag_sch2_active = !flag_sch2_active;
              break;
            default:
              break;
        }
      }
      else if (key == 'A') {
        index_schedule_keypad = 0;
        switch(status) {
            case SCHEDULE0_MODE:
                  status = SCHEDULE0_ACTIVE;
                  setActiveSchedule(0, flag_sch0_active);
                  break;
            case SCHEDULE0_ACTIVE:
                  status = SCHEDULE1_MODE;
                  displayTimeSchedule_LCD(setSchedule1);
                  break;
            case SCHEDULE1_MODE:
                  status = SCHEDULE1_ACTIVE;
                  setActiveSchedule(1, flag_sch1_active);
                  break;
            case SCHEDULE1_ACTIVE:
                  status = SCHEDULE2_MODE;
                  displayTimeSchedule_LCD(setSchedule2);
                  break;
            case SCHEDULE2_MODE:
                  status = SCHEDULE2_ACTIVE;
                  setActiveSchedule(2, flag_sch2_active);
                  break;
            case SCHEDULE2_ACTIVE:
                  status = SCHEDULE0_MODE;
                  displayTimeSchedule_LCD(setSchedule0);
                  break;
            default:
                break; 
        }  
        return;
      }
      if (status == SCHEDULE0_MODE || status == SCHEDULE1_MODE || status == SCHEDULE2_MODE)
          if (key > '9' || key < '0') return;

      if (index_schedule_keypad >= 6) index_schedule_keypad = 0;
      switch(status) {
          case SCHEDULE0_MODE:
                setSchedule0[index_schedule_keypad] = key;
                checkValidSchedule(setSchedule0);
                index_schedule_keypad++;
                displayTimeSchedule_LCD(setSchedule0);
                break;
          case SCHEDULE1_MODE:
                setSchedule1[index_schedule_keypad] = key;
                checkValidSchedule(setSchedule1);
                index_schedule_keypad++;
                displayTimeSchedule_LCD(setSchedule1);
                break;
          case SCHEDULE2_MODE:
                setSchedule2[index_schedule_keypad] = key;
                checkValidSchedule(setSchedule2);
                index_schedule_keypad++;
                displayTimeSchedule_LCD(setSchedule2);
                break;  
          case SCHEDULE0_ACTIVE:
              setActiveSchedule(0, flag_sch0_active);
              break;
            case SCHEDULE1_ACTIVE:
              setActiveSchedule(1, flag_sch1_active);
              break;
            case SCHEDULE2_ACTIVE:
              setActiveSchedule(2, flag_sch2_active);
              break; 
          default:
                
                break; 
      }  
  }
}
void action3() {
  char key = customKeypad.getKey();// Read the key
  if (key){
    if (index_foodisreleased >= 3)  index_foodisreleased = 0;

    if (key == 'A') return;
    else if (key == 'C') index_foodisreleased--;
    else foodReleasedEachTime_array[index_foodisreleased++] = key;

    displayFoodReleased();
  }
}
void action4() {
    char key = customKeypad.getKey();// Read the key
    if (key){
    if (index_maxfood >= 4)  index_maxfood = 0;

    if (key == 'A') return;
    else if (key == 'C') index_maxfood--;
    else MAX_FOOD_PER_DAY_array[index_maxfood++] = key;

    displayMaxFood();
  }
}
void action5() {
    clearSettings();
}