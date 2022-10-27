// // #include <Q2HX711.h>

// // const byte hx711_data_pint = 2;
// // const byte hx711_clock_pint = 3;
// // Q2HX711 hx711(hx711_data_pint, hx711_clock_pint);
// // long zero = 84172;
// // void setup() {
// //   // put your setup code here, to run once:
// //   Serial.begin(9600);
// // }

// // void loop() {
// //   // put your main code here, to run repeatedly:
// //   Serial.println(((hx711.read()/100) - zero)/38*10); // 0g: 84172
// //     // 0g 84172
// //     // 10g 84210
// //     //   84210 - 84172 = 38
// // }
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include "HX711.h"
// HX711 circuit wiring
HX711 current_food;
HX711 amount_of_remaining_food;
int readingCurrFood;
int lastReadingCurrFood;
int readingRemainFood;
int lastReadingRemainFood;
int CURRFOOD_DOUT_PIN = 2;
int CURRFOOD_SCK_PIN = 3;
int REMAINFOOD_DOUT_PIN = 4;
int REMAINFOOD_SCK_PIN = 5;
int motorPin1 = 6;
int motorPin2 = 7;

int setting_food = 0;
bool feed_active = true;
float CALIBRATION_FACTOR = -100; // weight / 1g

void setup()
{
    Serial.begin(57600);
    setting_food = 5000;
    pinMode(motorPin1,OUTPUT) ; 
    pinMode(motorPin2,OUTPUT) ;    

    // //This code  will turn Motor A clockwise(in) for 2 sec.
    // digitalWrite(motorPin1, LOW);
    // digitalWrite(motorPin2, HIGH);
    // delay(2000);

    // //This code will turn Motor A counter-clockwise (out) for 2 sec.
    // digitalWrite(motorPin1, HIGH);
    // digitalWrite(motorPin2, LOW);
    // delay(2000);


    // // turn off motor
    // digitalWrite(motorPin1, LOW);
    // delay(2000);


    // //This code  will turn Motor A clockwise(in) for 4 sec.
    // digitalWrite(motorPin1, LOW);
    // digitalWrite(motorPin2, HIGH);
    // delay(4000);

    // //This code will turn Motor A counter-clockwise (out) for 2 sec.
    // digitalWrite(motorPin1, HIGH);
    // digitalWrite(motorPin2, LOW);
    // delay(2000);

    // // turn off motor
    // digitalWrite(motorPin1, LOW);
    // delay(2000);

    // Serial.println("HX711");
    // Serial.println("Initializing the scale");    
    // current_food.begin(CURRFOOD_DOUT_PIN, CURRFOOD_SCK_PIN);

    // Serial.println("Before setting up the scale:");
    // Serial.print("read: \t\t");
    // Serial.println(current_food.read());      // print a raw reading from the ADC

    
    // Serial.print("read average: \t\t");
    // Serial.println(current_food.read_average(20));   // print the average of 20 readings from the ADC
  
    // Serial.print("get value: \t\t");
    // Serial.println(current_food.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)
  
    // Serial.print("get units: \t\t");
    // Serial.println(current_food.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
    //           // by the SCALE parameter (not set yet)
              
    current_food.begin(CURRFOOD_DOUT_PIN, CURRFOOD_SCK_PIN);
    current_food.set_scale(CALIBRATION_FACTOR);
    current_food.tare();               // reset the scale to 0
    amount_of_remaining_food.begin(REMAINFOOD_DOUT_PIN, REMAINFOOD_SCK_PIN);
    amount_of_remaining_food.set_scale(CALIBRATION_FACTOR);
    amount_of_remaining_food.tare(); 
       
    // Serial.println("After setting up the scale:");
  
    // Serial.print("read: \t\t");
    // Serial.println(current_food.read());                 // print a raw reading from the ADC
  
    // Serial.print("read average: \t\t");
    // Serial.println(current_food.read_average(20));       // print the average of 20 readings from the ADC
  
    // Serial.print("get value: \t\t");
    // Serial.println(current_food.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()
  
    // Serial.print("get units: \t\t");
    // Serial.println(current_food.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
    //           // by the SCALE parameter set with set_scale
  
    // Serial.println("Readings:");
  
}

// rotate when food_current < food set up
void rotateMotor(){
  if(readingCurrFood < setting_food ){
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  }
  else{
    digitalWrite(motorPin1, LOW);
  }
}

void loop()
{
  if (current_food.wait_ready_timeout(200)) {
    readingCurrFood = round(current_food.get_units());
    Serial.print("Weight: ");
    Serial.println(readingCurrFood);
    if (readingCurrFood != lastReadingCurrFood){
      
      Serial.print("Weight: ");
      Serial.println(readingCurrFood);
    }
    lastReadingCurrFood = readingCurrFood;
  }
  else {
    Serial.println("HX711 not found.");
  }
  rotateMotor();

  delay(1000);
}
