// AUTOMATIC PET FEEDER
// HOW TO USE?
// Button0: Press once to switch between food remains and food in container and time to feed information.
// Button1: Setup time to feed your pet. Hold the button to switch between states (hour->minute->second->hour1->,..->hour->minute->,..). It depends on your current state.
// Button2: Set the maximum amount of food to feed per day (it can only be set to 1600g. Press once to increase 200 and hold the button to decrease the value by 200.
// Button3: Reset all settings.
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>

#define IS_PRESSED  HIGH
#define RP_MODE      1      // display remaining food in the plate on LCD
#define RC_MODE      2      // display remaining food in the container on LCD
#define T_MODE       3      // display time to feed on LCD
#define INIT_MODE    4
#define H_MODE       5      // Set schedule for hour0
#define M_MODE       6      // Set schedule for minute0
#define S_MODE       7      // Set schedule for second0
#define H1_MODE      8      // Set schedule for hour1
#define M1_MODE      9      // Set schedule for minute1
#define S1_MODE      10     // Set schedule for second1
#define H2_MODE      11     // Set schedule for hour2
#define M2_MODE      12     // Set schedule for minute2
#define S2_MODE      13     // Set schedule for second2
#define TIME_NOTHING_CHANGES  10000 // If nothing changes after 10s, then turn off LCD to save power

// Declare for any var about time here.
unsigned long startMillis;  // Some global variables available anywhere in the program
unsigned long currentMillis; // Current time since the program started 
int hour[3], minute[3], second[3];
// Declare for any flag here.
bool flag_longclick_btn1;
bool flag_settingSchedule;
bool flag_duringLongPress;
bool flag_settingMaxFood;
// Declare for the button or something else. 
OneButton button1(51, LOW);
OneButton button2(49, LOW);
OneButton button3(47, LOW);
int btn[4] = {53, 51, 49, 47};
int MAX_FOOD_PER_DAY;
int remaining_food;
int remainingFoodInContainer;
int mode;
int counter;
int status;

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// Display the remaining food on the LCD. 
void displayRemainingFood_LCD(int status) {
    lcd.init();                      // Initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(1,0);
    lcd.print("REMAINING FOOD");

    // Display (g) is still on the plate. 
    int count = 0;
    int temp = remaining_food;
    do {
      temp /= 10;
      ++count;
    } while (temp != 0);
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
void displayTimeSchedule_LCD(int hour, int minute, int second) {
    int current_cursor = 0;
    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(2,0);
    if (flag_settingSchedule) {
        lcd.setCursor(3,0);
        switch(status) {
            case H_MODE:
                 lcd.print("SET HOUR_0");
                 break;
            case M_MODE:
                 lcd.print("SET MINUTE_0");
                 break;
            case S_MODE:
                 lcd.print("SET SECOND_0");
                 break;
            case H1_MODE:
                 lcd.print("SET HOUR_1");
                 break;
            case M1_MODE:
                 lcd.print("SET MINUTE_1");
                 break;
            case S1_MODE:
                 lcd.print("SET SECOND_1");
                 break;  
            case H2_MODE:
                 lcd.print("SET HOUR_2");
                 break;
            case M2_MODE:
                 lcd.print("SET MINUTE_2");
                 break;
            case S2_MODE:
                 lcd.print("SET SECOND_2");
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
void displayMaxFood() {
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("MAX FOOD PER DAY");
    lcd.setCursor(6,1);
    lcd.print(MAX_FOOD_PER_DAY);
}
void IncreaseMaxFood() {
    if (MAX_FOOD_PER_DAY >= 1600) {
        Exceeding(); // Notify the user if the maximum amount of food to feed is exceeded.
        return;
    }
    else MAX_FOOD_PER_DAY += 200;
    displayMaxFood();
}
void DecreaseMaxFood() {
  if (MAX_FOOD_PER_DAY <= 0) {
      ReachMin(); //Notify the user if the maximum amount of food to feed is equal to zero. 
      return;
  }
  else {
    MAX_FOOD_PER_DAY -= 200;
    displayMaxFood();
  }
  
}
// Don't allow the user to set more food per day (reach max).
void Exceeding() {
    lcd.init();
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("CAN'T INCREASE");
    lcd.setCursor(6, 1);
    lcd.print("MORE");
}
// Don't allow the user to set less food per day (reach min).
void ReachMin(){
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("CAN'T DECREASE");
    lcd.setCursor(6, 1);
    lcd.print("MORE");
}
void setup()
{
    Serial.begin(115200);
    pinMode(btn[0], INPUT);
    pinMode(btn[1], INPUT);
    pinMode(btn[2], INPUT);
    pinMode(btn[3], INPUT);
    remaining_food = 0;
    MAX_FOOD_PER_DAY = 0;
    hour[3] = {0}, minute[3] = {0}, second[3] = {0};
    flag_longclick_btn1 = 0;
    flag_settingSchedule = 0;
    flag_duringLongPress = 0;
    flag_settingMaxFood = 0;
    status = H_MODE;
    mode = RC_MODE;
    
    // Functions of button1.
    button1.attachClick(click1);                     // Fires as soon as a single click of button1 is detected.
    button1.attachDuringLongPress(duringLongPress);  // Fires periodically as long as the button1 is held down.
    button1.attachLongPressStop(stopLongPress);      // Fires when the button1 is released after a long hold.
    button1.setPressTicks(1000);                     // Set duration to hold a button1 to trigger a long press.
    // Functions of button2.
    button2.attachClick(IncreaseMaxFood);
    button2.attachDuringLongPress(DecreaseMaxFood);
    button1.setPressTicks(1000);

    button3.attachClick(clearSettings);
    startMillis = millis();                          // Initial start time.
}

void clearSettings() {
    MAX_FOOD_PER_DAY = 0;
    hour[3] = {0}, minute[3] = {0}, second[3] = {0};
    if (flag_settingMaxFood) {
        displayMaxFood(); 
    }
}
void loop()
{
  button1.tick();
  button2.tick();
  button3.tick();
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
    case RC_MODE: 
        displayRemainingFood_LCD(RC_MODE);
        mode = RP_MODE;
        break;
    case RP_MODE: 
        displayRemainingFood_LCD(RP_MODE);
        mode = T_MODE;
        break;
    case T_MODE: 
        displayTimeSchedule_LCD(hour, second, minute);
        mode = RC_MODE;
        break;
    default:
        lcd.clear();
    }
  }
  // Setup schedule with button 2
  // If you hold the button for at least 1 seconds, then set for an hour/minute/second depending on the current state.
  
  if (digitalRead(btn[1]) == IS_PRESSED) {
      flag_settingMaxFood = 0;
      flag_settingSchedule = 1;
  }
  // Setup max food per day, with a max initial value of 0. To increase the 200, press once.
  if (digitalRead(btn[2]) == IS_PRESSED) {
      flag_settingSchedule = 0;
      flag_settingMaxFood = 1;
  }
  delay(1);
}

void duringLongPress() {
     flag_duringLongPress = 1;   
}

void stopLongPress() {
      flag_duringLongPress = 0;
      switch(status) {
        case INIT_MODE:
                 status = H_MODE;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;      
        case H_MODE:
                 status = M_MODE;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case M_MODE: 
                 status = S_MODE;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case S_MODE:
                 status = H1_MODE;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case H1_MODE:
                 status = M1_MODE;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                 break;
        case M1_MODE: 
                 status = S1_MODE;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                  break;
        case S1_MODE:
                 status = H2_MODE;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                 break;
        case H2_MODE:
                 status = M2_MODE;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        case M2_MODE: 
                 status = S2_MODE;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        case S2_MODE:
                 status = H_MODE;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        default:
                 break;
      }
}
void click1() {
  if (!flag_duringLongPress) {
      switch(status) {
        case H_MODE:
                 if (hour[0] >= 24) hour[0] = 0;
                 else hour[0]++;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case M_MODE: 
                 if (minute[0] >= 60) minute[0] = 0;
                 else minute[0]++;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case S_MODE:
                 if (second[0] >= 60) second[0] = 0;
                 else second[0]++;
                 displayTimeSchedule_LCD(hour[0], minute[0], second[0]);
                 break;
        case H1_MODE:
                 if (hour[1] >= 24) hour[1] = 0;
                 else hour[1]++;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                 break;
        case M1_MODE: 
                 if (minute[1] >= 60) minute[1] = 0;
                 else minute[1]++;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                  break;
        case S1_MODE:
                 if (second[1] >= 60) second[1] = 0;
                 else second[1]++;
                 displayTimeSchedule_LCD(hour[1], minute[1], second[1]);
                 break;
        case H2_MODE:
                 if (hour[2] >= 24) hour[2] = 0;
                 else hour[2]++;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        case M2_MODE: 
                 if (minute[2] >= 60) minute[2] = 0;
                 else minute[2]++;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        case S2_MODE:
                 if (second[2] >= 60) second[2] = 0;
                 else second[2]++;
                 displayTimeSchedule_LCD(hour[2], minute[2], second[2]);
                 break;
        default:
                 lcd.init();
                 lcd.clear();
                 lcd.setCursor(2, 0);
                 lcd.print("CAN'T DETECT");
                 lcd.setCursor(4, 1);
                 lcd.print("A STATE");
                 break;
      }
  }
}
