
#include <Firebase_ESP_Client.h>
//#include <FirebaseESP8266.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Tien"
#define WIFI_PASSWORD "26122002"
// #define FIREBASE_HOST "pet-app-fd9d6-default-rtdb.firebaseio.com"
// new test
#define DATABASE_URL "pet-app-fd9d6-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyCkD_bTTme64MFXSyu6CzZmy-pTwhMvyDk"
//
#define FIREBASE_AUTH "bZjJFG5UBlzLiiBfLB4Aj448icxuWbxoTOXkY85i"
#define FIREBASE_PROJECT_ID "pet-app-fd9d6"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String dulieu;
String data;
int wifi_index = 0;
int received_data[5] = {0};
int intValue;
float floatValue;
bool signupOK = false;
SoftwareSerial ESP8266_softSerial(13, 12);

void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void setup() {
  Serial.begin(57600);
  ESP8266_softSerial.begin(9600);
  initWifi();
}

void receiveDataFromArduino() {

}
void loop() {
    /* ESP8266 Received msg from Arduino */

    while(ESP8266_softSerial.available()) {
      data = ESP8266_softSerial.readStringUntil('\n');
      /* Remove end line */
      data.remove(data.length() - 1, 1);
      /* Check if arduino has received data; if so, delete node on firebase */
      if (data == "data 0") received_data[0] = true;
      else if (data == "data 1") received_data[1] = true;
      else if (data == "data 2") received_data[2] = true;
      else if (data == "data 3") received_data[3] = true;
      else if (data == "data 4") received_data[4] = true;
      else if (data[0] == 'F' && data[1] == 'S') {
          // FS321 2012-02-27 13:27:00
          /* Remove recognise character FS*/
          data.remove(0, 2);
          Serial.println(data);
          String weight_str = data;
          // Get weight and time from string
          int i = 0;
          for (; i < data.length() - 1; i++) if (data[i] == ' ') break;
          weight_str.remove(i, data.length() - 1);
          double weight = weight_str.toInt();
          data.remove(0, i + 1);
          firestoreDataUpdate(weight, data);
      }
      data = "";
    }
        
    /* Get data from firebase and send to Arduino */
    if (wifi_index == 0) {
      if(Firebase.RTDB.getString(&fbdo, "/hand_push/push"))
      {
        dulieu = fbdo.stringData();
        dulieu = "R" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        Serial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.RTDB.getString(&fbdo, "/pet_app_demo/sche0"))
      {
        dulieu = fbdo.stringData();
        dulieu = "X" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        Serial.println(dulieu);
        dulieu = "";
      }
      
      if(Firebase.RTDB.getString(&fbdo, "/pet_app_demo/sche1"))
      {
        dulieu = fbdo.stringData();
        dulieu = "Y" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        Serial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.RTDB.getString(&fbdo, "/pet_app_demo/sche2"))
      {
        dulieu = fbdo.stringData();
        //dulieu.remove(0,2);
        dulieu = "Z" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        Serial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.RTDB.getString(&fbdo, "/pet_app_demo/maxfood"))
      {
        dulieu = fbdo.stringData();
        dulieu = "M" + dulieu;
        ESP8266_softSerial.println(dulieu);
        Serial.println(dulieu);
        dulieu = "";
        wifi_index = 1;
      }
      
    } else if (wifi_index == 1) { /* Delete Node if arduino is received */
      if (received_data[0]) {
        Serial.println("Delete 0");
        received_data[0] = false;
        Firebase.RTDB.deleteNode(&fbdo, "/hand_push/push");
      }
      if (received_data[1]) {
        Serial.println("Delete 1");
        received_data[1] = false;
        Firebase.RTDB.deleteNode(&fbdo, "/pet_app_demo/sche0");
      }
      if (received_data[2]) { 
        Serial.println("Delete 2");
        received_data[2] = false;
        Firebase.RTDB.deleteNode(&fbdo, "/pet_app_demo/sche1");
      }
      if (received_data[3]) {
        Serial.println("Delete 3");
        received_data[3] = false;
        Firebase.RTDB.deleteNode(&fbdo, "/pet_app_demo/sche2");
      }
      if (received_data[4]) {
        received_data[4] = false;
        Firebase.RTDB.deleteNode(&fbdo, "/pet_app_demo/maxfood");
      }
      wifi_index = 0; 
    }
    delay(400);
}

// generate random string of length 20
String randomString() {
  // character 33 -> 94 in ascii
  String res = "";
  for (int i = 0; i < 10; i++) res += '!' + rand() % 222;
  return res;
}

void firestoreDataUpdate(double weight, String time){
  if(WiFi.status() == WL_CONNECTED && Firebase.ready()){
    String randomID = randomString();
    String documentPath = "history/" + randomID;

    FirebaseJson content; 

    content.set("fields/weight/doubleValue", String(weight).c_str());
    content.set("fields/time/stringValue", String(time).c_str());

    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "time,weight")){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    } else{
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }
  }
}

