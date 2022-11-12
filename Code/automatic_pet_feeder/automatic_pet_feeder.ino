// AUTOMATIC PET FEEDER
// HOW TO USE?
// Button0: Press once to switch between food remains and food in container and time to feed information.
// Button1: Setup time to feed your pet. Hold the button to switch between states (hour->minute->second->hour1->,..->hour->minute->,..). It depends on your current state.
// Button2: Set the maximum amount of food to feed per day (it can only be set to 1600g). Press once to increase 200 and hold the button to decrease the value by 200.
// Button3: Set the food is released each time (it can only be set to 1600g). Press once to increase 10 and hold the button to decrease the value by 10.
// Button4: Reset all settings. If you press once, a notification will be shown to confirm; if you press again, settings will be reset.
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include "Key.h"
#include <Keypad_I2C.h>
#include <Keypad.h>        // GDY120705
#include <Wire.h>
#include <DS1302.h>
#include "HX711.h"
#include <Servo.h>

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

#define TIME_NOTHING_CHANGES    30000           // If nothing changes after 30S, then turn off LCD to save power
#define MAX_FOOD                1600            // Per day food allowance.
#define MIN_FOOD                0
#define MAX_TIMES_FOOD_RELEASED 3
/* Touch sensor */
#define TOUCH_SENSOR A0
/* KeyPad define */
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

/* Real time clock */
DS1302 rtc(2, 4, 5); //RST,DAT,CLK Pins of the DS1302 Module 

/* Declare for any var about time here. */
unsigned long startMillis;        // Some global variables available anywhere in the program
unsigned long currentMillis;      // Current time since the program started 
unsigned long startMillisTouch;
unsigned long startMillisReceiveData;
/* Declare for any flag here. */
bool flag_settingSchedule;
bool flag_settingMaxFood;
bool flag_sch0_active;             // If flag = 1 then feed the animal according to the time0 and vice versa.
bool flag_sch1_active;            // If flag = 1 then feed the animal according to the time1 and vice versa.
bool flag_sch2_active;            // If flag = 1 then feed the animal according to the time2 and vice versa.
bool FIRST_PRESS_RESET;
bool flag_confirm = false;
bool flag_confirm_with_keypad = false;


/* Declare for the button or something else. */

int btn[4] = {9, 10, 11, 12};
int mode = RC_MODE;                      // State to display on LCD (for button0).
int counter;                             // Variable for calculating the total number of digits.
int status = INIT_MODE;                   
int menu = 1;
// Declare about food here
int remaining_food = 0;
int remainingFoodInContainer;
bool whileDisplayRealTimeClock = 1;
int index_schedule_keypad = 0;
int index_foodisreleased = 0;
int index_maxfood = 0;
int current_index_action3 = 0;
int index_action1 = 0;
/* Char array for store all datas of time/ food */
struct foodReleased {
  char food[3] = {'1', '5', '0'};
};

struct indexOfCharKeypad {
  int releasedFood[3];
  int flagReleasedFood[3];
  int flagmaxFood;
  int maxFood;
};

indexOfCharKeypad indexKeypad;
foodReleased foodReleasedEachTime_array[3];
char MAX_FOOD_PER_DAY_array[4] = {'1', '0', '0', '0'};
char setSchedule0[6] = {'0', '0', '0', '0', '0', '0'};
char setSchedule1[6] = {'0', '0', '0', '0', '0', '0'};
char setSchedule2[6] = {'0', '0', '0', '0', '0', '0'};

LiquidCrystal_I2C lcd(LCDADDR,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
// weight sensor
HX711 current_food;
HX711 amount_of_remaining_food;
double readingCurrFood;
double lastReadingCurrFood;
double readingRemainFood;
double lastReadingRemainFood;
int CURRFOOD_DOUT_PIN = 2;
int CURRFOOD_SCK_PIN = 3;
int REMAINFOOD_DOUT_PIN = 4;
int REMAINFOOD_SCK_PIN = 5;
int motorPin1 = 6;
int motorPin2 = 7;
int setting_food = 0;
bool feed_active = false;
float CALIBRATION_FACTOR = -100; // weight / 1g

// servo
Servo myservo;  // create servo object to control a servo
// update menu for lcd
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
    case 7:
      menu = 6;
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
              lcd.print(" - def");
              break;              
          }
    }
    // Display hour
    for (int i = 0; i < strlen(setSchedule); i++) Serial.println(setSchedule[i]);

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

void displayFoodReleased(int index) {
    lcd.init();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FOOD IS RELEASED");
    lcd.setCursor(0,1);
    lcd.print("EACH TIME ");
    lcd.setCursor(10, 1);

    lcd.print(index);
    lcd.setCursor(11, 1);
    lcd.print(":");
    lcd.setCursor(12, 1);
    int current_cursor = 12;
    int temp = current_cursor;
    int indexReleased;
    if (indexKeypad.flagReleasedFood[index] == 1) {
      indexReleased = 4;
    } else indexReleased = indexKeypad.releasedFood[index] + 1;
    if (indexKeypad.releasedFood[index] == 0) {
      lcd.print(0);
      temp = 1;
    } else{ 
        for (int i = 0; i < indexReleased; i++ ) {
          if (foodReleasedEachTime_array[index].food[i] > '9' || foodReleasedEachTime_array[index].food[i] < '0') break;
          lcd.print(foodReleasedEachTime_array[index].food[i]);
          temp = i;
        }
    }
    //Serial.println(current_cursor + temp);
    lcd.setCursor(current_cursor + temp, 1);
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
void backFunction() {
  switch(menu) {
    case 1:
      switch(mode) {
          case RC_MODE:
             // displayRemainingFood_LCD(RC_MODE);
              mode = T2_MODE;
              displayTimeSchedule_LCD(setSchedule2); 
              break;
          case RP_MODE: 
              mode = RC_MODE;
              displayRemainingFood_LCD(RC_MODE);
              break;
          case RF_MODE:
              mode = RP_MODE;
              displayRemainingFood_LCD(RP_MODE);
              break;
          case T0_MODE:      
              mode = RF_MODE;
              displayFoodReleased(0);
              break;
          case T1_MODE:  
              mode = T0_MODE;
              displayTimeSchedule_LCD(setSchedule0);
              break;
          case T2_MODE:
              mode = T1_MODE;
              displayTimeSchedule_LCD(setSchedule1);
              break;
          default:
            break;
      }
      break;
    case 2:
      switch(status) {
        case SCHEDULE0_MODE:
              status = SCHEDULE2_ACTIVE;
              setActiveSchedule(2, flag_sch2_active);
              break;
        case SCHEDULE0_ACTIVE:
              status = SCHEDULE0_MODE;
              displayTimeSchedule_LCD(setSchedule0);
              break;
        case SCHEDULE1_MODE:
              status = SCHEDULE0_ACTIVE;
              setActiveSchedule(0, flag_sch0_active);
              break;
        case SCHEDULE1_ACTIVE:
              status = SCHEDULE1_MODE;
              displayTimeSchedule_LCD(setSchedule1);
              break;
        case SCHEDULE2_MODE:
              status = SCHEDULE1_ACTIVE;
              setActiveSchedule(1, flag_sch1_active);
              break;
        case SCHEDULE2_ACTIVE:
              status = SCHEDULE2_MODE;
              displayTimeSchedule_LCD(setSchedule2);
              break;
        default:
            break; 
      } 
      break;
    default:
      Serial.println(menu);
      break;
  }
}
// void initFoodReleased() {
//   for (int i = 0; i < MAX_TIMES_FOOD_RELEASED; i++)
//     foodReleasedEachTime_array[i].index_action1 = 0;
// }
void initIndexOfCharKeypad() {
  indexKeypad.maxFood = 0;
  indexKeypad.flagmaxFood = 0;

  for (int i = 0; i < 3; i++) {
    indexKeypad.releasedFood[i] = 0;
    indexKeypad.flagReleasedFood[i] = 0;
  }
}
void setup()
{ 
    
    Serial.begin(9600);
    // setup for real time clock
    rtc.halt(false);
    rtc.writeProtect(false);
    lcd.backlight();
    // init for keypad and button
    //Wire.setClock(10000);
    lcd.home();
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.begin(16, 2);
    keypad.begin(makeKeymap(keys));
    Wire.begin();

    pinMode(btn[0], INPUT_PULLUP);
    pinMode(btn[1], INPUT_PULLUP);
    pinMode(btn[2], INPUT_PULLUP);
    pinMode(btn[3], INPUT_PULLUP);
    pinMode(btn[4], INPUT_PULLUP);
    flag_sch0_active = 0;
    flag_sch1_active = 0;
    flag_sch2_active = 0;
    FIRST_PRESS_RESET = 1; 
    // Initial start time.
    startMillis = millis();   
    startMillisTouch = millis();       
    startMillisReceiveData = millis();     

    // setup for weight sensor
    lcd.setCursor(1,0);
    lcd.print("Initializing");
    lcd.setCursor(1, 1);
    lcd.print("machine...");
    Serial.println("Initializing the scale");    
    current_food.begin(CURRFOOD_DOUT_PIN, CURRFOOD_SCK_PIN);

    Serial.println("Before setting up the scale:");
    Serial.print("read: \t\t");
    Serial.println(current_food.read());      // print a raw reading from the ADC

    Serial.print("read average: \t\t");
    Serial.println(current_food.read_average(20));   // print the average of 20 readings from the ADC
  
    Serial.print("get value: \t\t");
    Serial.println(current_food.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)
  
    Serial.print("get units: \t\t");
    Serial.println(current_food.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
              // by the SCALE parameter (not set yet)
    current_food.begin(CURRFOOD_DOUT_PIN, CURRFOOD_SCK_PIN);
    current_food.set_scale(111.f);
    current_food.tare();               // reset the scale to 0

    amount_of_remaining_food.begin(REMAINFOOD_DOUT_PIN, REMAINFOOD_SCK_PIN);
    amount_of_remaining_food.set_scale(111.f);
    amount_of_remaining_food.tare(); 
    Serial.println("After setting up the scale:");
  
    Serial.print("read: \t\t");
    Serial.println(current_food.read());                 // print a raw reading from the ADC
  
    Serial.print("read average: \t\t");
    Serial.println(current_food.read_average(20));       // print the average of 20 readings from the ADC
  
    Serial.print("get value: \t\t");
    Serial.println(current_food.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()
  
    Serial.print("get units: \t\t");
    Serial.println(current_food.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
    // by the SCALE parameter set with set_scale
  
    Serial.println("Readings:");
    // Servo
    myservo.attach(7);

    //initFoodReleased(); 
    initIndexOfCharKeypad();
    updateMenu();
    
}

int wifi_test = 0;
String data;
int pos = 0;
void updateChar(char *arr, String data, int size) {
  for (size_t i = 0; i < size; i++) arr[i] = data[i];
}
void loop() {
  // weight sensor
  // if (current_food.wait_ready_timeout(200)) {
  // readingRemainFood = round(current_food.get_units());

  // if (readingRemainFood != lastReadingRemainFood){
    
  //   Serial.print("Weight: ");
  //   Serial.println(readingRemainFood);
  // }
  //   lastReadingRemainFood = readingRemainFood;
  // }
  
  // for (int pos = 180; pos > 0; pos--) {
  //     if (readingRemainFood >= 10) {
  //       myservo.write(pos);
  //     } else break;
  // }                               // mỗi bước của vòng lặp tăng 1 độ
    // if (readingRemainFood >= 10) {
    //   if (pos < 180 && pos > 0) myservo.write(pos--); 
    //   else myservo.write(pos++); 
    // }
    
//   for(int pos = 180; pos>=1; pos-=1)     // cho servo quay từ 179-->0 độ
//   {                                
//     if (readingRemainFood >= 10)  
//     else break;  
//  } 

  whileDisplayRealTimeClock = false;
  
  flag_confirm = false;
  currentMillis = millis();  // Get the current "time" (actually the number of milliseconds since the program started)
  // Turn off LCD after 15s if nothing changes
  if ( analogRead(TOUCH_SENSOR) > 650 && (currentMillis - startMillisTouch >= 80) ) {
    Serial.print("touch!!");
    startMillisTouch = millis();
  }
  while(Serial.available()) {
    data = Serial.readStringUntil('\n');
  }
  if (data[0] == 'R') {
    data.remove(0, 1);
    //updateChar(foodReleasedEachTime_array, data, 3);
    data = "";
    //delay(60);
  } else if (data[0] == 'X') {
    data.remove(0, 1);
    updateChar(setSchedule0, data, 6);
    data = "";
    //delay(60);
  } else if (data[0] == 'Y') {
    data.remove(0, 1);
    updateChar(setSchedule1, data, 6);
    data = "";
    //delay(60);
  } else if (data[0] == 'Z') {
    data.remove(0, 1);
    updateChar(setSchedule2, data, 6);
    data = "";
    //delay(60);
  } else if (data[0] == 'M') {
    data.remove(0, 1);
    updateChar(MAX_FOOD_PER_DAY_array, data, 4);
    data = "";
    //delay(60);
  }

  //Serial.println()
  if (currentMillis - startMillis >= TIME_NOTHING_CHANGES) {
      lcd.noBacklight();
      lcd.noDisplay();   
  } else {
    lcd.backlight();
    lcd.display();
  }
  /* If any button is pressed, then start calculate time for turn off LCD again. */
  if ((!digitalRead(btn[0]) || !digitalRead(btn[1]) || !digitalRead(btn[2]) || !digitalRead(btn[3]) 
                            || flag_confirm || flag_confirm_with_keypad) ) startMillis = millis();
  /* Down button */
  if (!digitalRead(btn[2])){
    flag_confirm = false;
    flag_confirm_with_keypad = false;
    if (menu >= 5) menu = 1;
    else menu++;
    updateMenu();

    while (!digitalRead(btn[2])) delay(50);
  }
  /* Up button */
  if (!digitalRead(btn[3])){
    flag_confirm = false;
    flag_confirm_with_keypad = false;
    if (menu <= 1) menu = 5;
    else menu--;
    updateMenu();
    
    while(!digitalRead(btn[3])) delay(50);;
  }
  /* Back button */
  if (!digitalRead(btn[1])) {
      flag_confirm = false;
      backFunction();
      while(!digitalRead(btn[1])) delay(50);;
  }
  /* Confirm button */
  if (!digitalRead(btn[0])){
      flag_confirm = true;
      index_schedule_keypad = 0;
      flag_settingSchedule = false;
      if (menu == 2) {
        flag_settingSchedule = true;
        flag_confirm_with_keypad = true;
        switch(status) {
          case INIT_MODE:
            status = SCHEDULE0_MODE;
            displayTimeSchedule_LCD(setSchedule0);
            break;
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
      } else if (menu == 3) {
        flag_settingSchedule = false;
        flag_confirm_with_keypad = true;
        displayFoodReleased(current_index_action3);
        // if (current_index_action3 >= 2) current_index_action3 = 0;
        // else current_index_action3++;
      } else if (menu == 4) {
          flag_settingSchedule = false;
          flag_confirm_with_keypad = true;
          displayMaxFood();
      } else if (menu == 5) {
          flag_settingSchedule = false;
          flag_confirm_with_keypad = false;
          printConfirm();       
      }
        
      while (!digitalRead(btn[0])) delay(50);
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
        setZero(MAX_FOOD_PER_DAY_array); 
        for (int i = 0; i < MAX_TIMES_FOOD_RELEASED; i++) setZero(foodReleasedEachTime_array[i].food);
        flag_sch0_active = 0, flag_sch1_active = 0, flag_sch2_active = 0;
        mode = RC_MODE;
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("DONE !!");
        FIRST_PRESS_RESET = 1;
    } 
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
            //Serial.println(index_action1);
            displayFoodReleased(index_action1);
            if (index_action1 >= 2) {
              index_action1 = 0;
              mode = T0_MODE;
            }
            else index_action1++;
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
            Serial.println(mode);
    }
}
void action2() {
  flag_settingSchedule = 1;
  char key = keypad.getKey();// Read the key
  //Serial.print(key);
  if (key) {
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
      else if (key == 'D') {
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
      } else if (key == 'C') {
        index_schedule_keypad = 0; 
        switch(status) {
          case SCHEDULE0_MODE:
                setZero(setSchedule0);
                displayTimeSchedule_LCD(setSchedule0);
                break;
          case SCHEDULE1_MODE:
                setZero(setSchedule1);
                displayTimeSchedule_LCD(setSchedule1);
                break;
          case SCHEDULE2_MODE:
                setZero(setSchedule2);
                displayTimeSchedule_LCD(setSchedule2);
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
  
  char key = keypad.getKey();// Read the key
  
  if (key){
    if (key == 'D') { 
      if (current_index_action3 >= 2) current_index_action3 = 0;
      else current_index_action3++;
      displayFoodReleased(current_index_action3);
      return;
    }
    else if (key == 'C') {
      for (int i = 0; i < MAX_TIMES_FOOD_RELEASED; i++) setZero(foodReleasedEachTime_array[i].food);
    }
    else if (key == 'A') {
      if (indexKeypad.releasedFood[current_index_action3] > 0)
        indexKeypad.releasedFood[current_index_action3]--;
    }
    else {
      foodReleasedEachTime_array[current_index_action3].food[indexKeypad.releasedFood[current_index_action3]++] = key;
    }
    if (indexKeypad.releasedFood[current_index_action3] >= 3)  indexKeypad.flagReleasedFood[current_index_action3] = 1;
    else indexKeypad.flagReleasedFood[current_index_action3] = 0;
    displayFoodReleased(current_index_action3);
  }
}
void action4() {
  char key = keypad.getKey();// Read the key
    if (key){

      if (key == 'D') return;
      else if (key == 'C') setZero(MAX_FOOD_PER_DAY_array);
      else if (key == 'A') indexKeypad.maxFood--;
      else MAX_FOOD_PER_DAY_array[indexKeypad.maxFood++] = key;

      if (indexKeypad.maxFood >= 4)  indexKeypad.flagmaxFood= 1;
      else indexKeypad.flagmaxFood = 0;
      
      displayMaxFood();
  }
}
void action5() {
    clearSettings();
}

void displayRealTimeClock()
{
  whileDisplayRealTimeClock = true;
  lcd.setCursor(0,1);
  lcd.print("Time:");
  lcd.setCursor(5, 1);
  lcd.print(rtc.getTimeStr());
  lcd.setCursor(0, 0);
  lcd.print("Date:");
  lcd.setCursor(5,0);
  lcd.print(rtc.getDateStr(FORMAT_SHORT, FORMAT_LITTLEENDIAN, '/'));

  delay (100); 
}
