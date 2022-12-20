#include "display_lcd.h"

int menu = 0;

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
    int count1 = calDigitofNumber(int(readingCurrFood));
    int count2 = calDigitofNumber(int(readingRemainFood));
    
    switch(status) {
      case RP_MODE:
          lcd.setCursor(1, 1);
          lcd.print("PLATE:");
          lcd.setCursor(8, 1);
          lcd.print(int(readingCurrFood));
          lcd.setCursor(8 + count1 + 1, 1);
          lcd.print("G");
          break;
      case RC_MODE:
          lcd.setCursor(1, 1);
          lcd.print("CONTAINER:");
          lcd.setCursor(11, 1);
          lcd.print(int(readingRemainFood));
          lcd.setCursor(11 + count2, 1);
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
    //for (int i = 0; i < strlen(setSchedule); i++) Serial.println(setSchedule[i]);

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
    int temp = 0;
    int indexMaxFood;
    if (indexKeypad.flagmaxFood == 1) {
      indexMaxFood = 4;
    } else indexMaxFood = indexKeypad.maxFood;
    if (indexKeypad.maxFood == 0) {
      lcd.print(0);
      temp = 1;
    } else {  
        for (int i = 0; i < indexMaxFood; i++ ) {
          if (MAX_FOOD_PER_DAY_array[i] > '9' || MAX_FOOD_PER_DAY_array[i] < '0') break;
          lcd.print(MAX_FOOD_PER_DAY_array[i]);
          temp = i;
        }
    }
    lcd.setCursor(current_cursor + temp + 2, 1);
    lcd.print("G");

}

void displayFoodReleased(int index) {
    int current_cursor = 0;
    lcd.init();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FOOD IS RELEASED");
    lcd.setCursor(0,1);
    if (index != 3) {
      lcd.print("EACH TIME ");
      lcd.setCursor(10, 1);
      lcd.print(index);
      current_cursor = 10;
      lcd.setCursor(++current_cursor, 1);
      lcd.print(":");
    } else {
      lcd.print("DAILY");
      current_cursor = 4;
      lcd.setCursor(++current_cursor, 1);
      lcd.print(": ");
      current_cursor = 6;
    }

    lcd.setCursor(++current_cursor, 1);
    int temp = 0;
    int indexReleased;
    if (indexKeypad.flagReleasedFood[index] == 1) {
      indexReleased = 3;
    } else indexReleased = indexKeypad.releasedFood[index];
        
    if (indexKeypad.releasedFood[index] == 0) {
      lcd.print(0);
      temp = 1;
    } else {  
        for (int i = 0; i < indexReleased; i++ ) {
          if (foodReleasedEachTime_array[index].food[i] > '9' || foodReleasedEachTime_array[index].food[i] < '0') break;

          lcd.print(foodReleasedEachTime_array[index].food[i]);
          temp = i;
        }
    }
    lcd.setCursor(current_cursor + temp + 1, 1);
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