#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <SafeString.h>
#include <Arduino.h>

#define WIFI_SSID "Tien"
#define WIFI_PASSWORD "26122002"
#define FIREBASE_HOST "pet-app-fd9d6-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "bZjJFG5UBlzLiiBfLB4Aj448icxuWbxoTOXkY85i"
FirebaseData fbdo;
String dulieu;
String data;
int wifi_index = 0;
int received_data[5] = {0};
SoftwareSerial ESP8266_softSerial(13, 12);

void setup() {
  Serial.begin(57600);
  ESP8266_softSerial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
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
      data = "";
    }
        
    /* Get data from firebase and send to Arduino */
    if (wifi_index == 0) {
      if(Firebase.getString(fbdo, "/hand_push/push"))
      {
        dulieu = fbdo.stringData();
        dulieu = "R" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/sche0"))
      {
        dulieu = fbdo.stringData();
        dulieu = "X" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        dulieu = "";
      }
      
      if(Firebase.getString(fbdo, "/pet_app_demo/sche1"))
      {
        dulieu = fbdo.stringData();
        dulieu = "Y" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/sche2"))
      {
        dulieu = fbdo.stringData();
        //dulieu.remove(0,2);
        dulieu = "Z" + dulieu;
        wifi_index = 1;
        ESP8266_softSerial.println(dulieu);
        dulieu = "";
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/maxfood"))
      {
        dulieu = fbdo.stringData();
        dulieu = "M" + dulieu;
        ESP8266_softSerial.println(dulieu);
        dulieu = "";
        wifi_index = 1;
      }
      
    } else if (wifi_index == 1) { /* Delete Node if arduino is received */
      if (received_data[0]) {
        Serial.println("Delete 0");
        received_data[0] = false;
        Firebase.deleteNode(fbdo, "/hand_push/push");
      }
      if (received_data[1]) {
        Serial.println("Delete 1");
        received_data[1] = false;
        Firebase.deleteNode(fbdo, "/pet_app_demo/sche0");
      }
      if (received_data[2]) { 
        Serial.println("Delete 2");
        received_data[2] = false;
        Firebase.deleteNode(fbdo, "/pet_app_demo/sche1");
      }
      if (received_data[3]) {
        Serial.println("Delete 3");
        received_data[3] = false;
        Firebase.deleteNode(fbdo, "/pet_app_demo/sche2");
      }
      if (received_data[4]) {
        received_data[4] = false;
        Firebase.deleteNode(fbdo, "/pet_app_demo/maxfood");
      }
      wifi_index = 0; 
    }
    delay(400);
}