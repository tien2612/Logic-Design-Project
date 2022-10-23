//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <Servo.h>
Servo motor;

#include <DS3231.h> 
DS3231  rtc(SDA, SCL);
Time  t;

#include "HX711.h"
HX711 current_food;
HX711 amount_of_remaining_food;

#define IS_PRESSED  HIGH
#define R_MODE      1     // display remaining food on LCD
#define T_MODE      2     // display time to feed on LCD
#define H_MODE      5     // Set schedule for hour
#define M_MODE      6     // Set schedule for minute
#define S_MODE      7     // Set schedule for second

int MAX_FOOD_PER_DAY = 0;
int remaining_food = 0;
int btn[2] = {53, 51};
int hour = 0, minute = 0, second = 0;
int mode = R_MODE;
int counter = 0;
int status = H_MODE;
extern volatile unsigned long timer0_millis;

int motor_pin = 47;
int sensor_touch = 52;
int set_interval = 49;
int set_food = 48;
int speaker = 45;
int check_remaining_food = 44;
// int amount_of_remaining_food = A0;
// HX711 circuit wiring
int CURRFOOD_DOUT_PIN = 43;
int CURRFOOD_SCK_PIN = 42;
int REMAINFOOD_DOUT_PIN = 41;
int REMAINFOOD_SCK_PIN = 40;

int setting_food = 0;
int stop_rotate = 90;
int start_rotate = 90;
bool feed_active = false;
int max_food = 0;
int amout_of_food = 0;
// bool state_touch = true;
int last_feed = 6;
int next_feed = 14;
int interval = 8;
int hour_sys, minute_sys, second_sys;


LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

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
    lcd.print("KG");
}
void displayTimeSchedule_LCD(int hour, int minute, int second) {
    int current_cursor = 0;
    lcd.init();                      // initialize the lcd 
    // Print a message to the LCD.
    lcd.clear();                    // Clear screen
    lcd.backlight();                // Turn on background light
    lcd.setCursor(2,0);
    lcd.print("TIME TO FEED");
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

void setup()
{
    motor.attach(motor_pin);
    motor.write(stop_rotate);
    pinMode(sensor_touch, INPUT);
    pinMode(set_interval, INPUT);
    pinMode(set_food, INPUT);
    pinMode(check_remaining_food, INPUT);        
    pinMode(speaker, OUTPUT);
    digitalWrite(speaker, LOW);
    rtc.begin();
    current_food.begin(CURRFOOD_DOUT_PIN, CURRFOOD_SCK_PIN);
    current_food.set_scale(-459.542);
    current_food.tare();               // reset the scale to 0
    amount_of_remaining_food.begin(REMAINFOOD_DOUT_PIN, REMAINFOOD_SCK_PIN);
    amount_of_remaining_food.set_scale(-459.542);
    amount_of_remaining_food.tare();    
    max_food = 500;    
    amout_of_food = 100;
      
    Serial.begin(115200);
    pinMode(btn[0], INPUT);
    pinMode(btn[1], INPUT);
    MAX_FOOD_PER_DAY = 1000;
    remaining_food = 100;
}

bool CheckButtonIsHoldMoreThan1s(int button) {
     if (digitalRead(button) == HIGH) counter++;
     if (counter >= 1000) {
        counter = 0;
        return true;
     }
     return false;
}

// rotate when food_current < food set up
void rotateMotor(){
  if(feed_active){    
    while(analogRead(current_food.read()) < setting_food){
      motor.write(start_rotate);
    }
    motor.write(stop_rotate);
    feed_active = false;
  }
}

void loop()
{
  t = rtc.getTime();
  hour_sys = t.hour;
  minute_sys = t.min;
  second_sys = t.sec;

  // // push food follow setted time
  // if(hour_sys == hour && minute_sys == minute && second_sys == second){           
  //   feed_active = true;
  //   last_feed = hour;
  //   analogWrite(speaker, 150);
  //   delay(200);
  //   digitalWrite(speaker, LOW);
  // }
  
  // push food after a interval time 
  if(hour_sys == next_feed){                
    feed_active = true;
    last_feed = hour_sys;
    next_feed = hour_sys + interval;         
    if(next_feed >= 23){                 
      next_feed = next_feed - 24;   
    }     
    analogWrite(speaker, 150);
    delay(200);
    digitalWrite(speaker, LOW);
  }

  // press sensor touch
  if(digitalRead(sensor_touch)){
    feed_active = true;
  }

  // set interval time
  if(digitalRead(set_interval) == IS_PRESSED){
    interval++;
    if(interval > 23){
      interval = 1;
    }
  }
  
  // set up amount of food will be pushed
  if(digitalRead(set_food) == IS_PRESSED){
    setting_food += amout_of_food;
    if(setting_food > max_food){
      setting_food = amout_of_food;
    }
  }

  // print remaining food
  if(digitalRead(check_remaining_food) == IS_PRESSED){
    Serial.println(amount_of_remaining_food.get_units(), 1); 
  }  
  
  // int food_remain = analogRead(A0);
  // int setting_food = analogRead(A1);
  rotateMotor();  
  
  long int timer0_millis = millis();  
  // Turn off LCD after 15s if nothing changes
  if (timer0_millis >= 15000) {
      lcd.noDisplay(); 
  }
  
  if (digitalRead(btn[0]) || digitalRead(btn[1]) == IS_PRESSED) timer0_millis = 0;
  // Switching text if button 1 is pressed
  if (digitalRead(btn[0]) == IS_PRESSED) {
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
  if (digitalRead(btn[1]) == IS_PRESSED) {
      counter++;
      // If you hold the button for at least 2 seconds, then set for an hour/minute/second depending on the current state.
      if (counter >= 1000) {
        counter = 0;
        switch(status) {
          case H_MODE:
               while (!CheckButtonIsHoldMoreThan1s(btn[1])) {
                   displayTimeSchedule_LCD(hour, minute, second);
                   if (hour >= 24) hour = 0;
                   else hour++;
                   status = M_MODE;
               }
               break;
          case M_MODE:
               while (!CheckButtonIsHoldMoreThan1s(btn[1])) {
                   displayTimeSchedule_LCD(hour, minute, second);
                   if (minute >= 60) minute = 0;
                   else minute++;
                   status = S_MODE;
               }
               break;
          case S_MODE:
               while (!CheckButtonIsHoldMoreThan1s(btn[1])) {
                   displayTimeSchedule_LCD(hour, minute, second);
                   if (second >= 60) second = 0;
                   else second++;
                   status = H_MODE;
               }
               break;
        }
      }
      switch(status) {
          case H_MODE:
               displayTimeSchedule_LCD(hour, minute, second);
               if (hour >= 24) hour = 0;
               else hour++;
               break;
          case M_MODE:
               displayTimeSchedule_LCD(hour, minute, second);
               if (minute >= 60) minute = 0;
               else minute++;
               break;
          case S_MODE:
               displayTimeSchedule_LCD(hour, minute, second);
               if (second >= 60) second = 0;
               else second++;
               break;
        }
  }
}
