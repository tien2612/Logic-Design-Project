#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>


#include <Arduino.h>

int led = 5;     // LED pin
int button = 12; // push button is connected
#define WIFI_SSID "Tien"
#define WIFI_PASSWORD "26122002"
#define FIREBASE_HOST "pet-app-fd9d6-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "bZjJFG5UBlzLiiBfLB4Aj448icxuWbxoTOXkY85i"
FirebaseData fbdo;
String dulieu;
int wifi_index = 0;
void setup() {
    Serial.begin(9600);

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
    if (wifi_index == 0) {
      if(Firebase.getString(fbdo, "/hand_push/push"))
      {
        dulieu = fbdo.stringData();
        dulieu.remove(0,2);
        dulieu.remove(dulieu.length()-2,2);
        dulieu = "R" + dulieu;
        Serial.println(dulieu);
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/sche0"))
      {
        dulieu = fbdo.stringData();
        dulieu.remove(0,2);
        dulieu.remove(dulieu.length()-2,2);
        dulieu = "X" + dulieu;
        Serial.println(dulieu);
      }
      
      if(Firebase.getString(fbdo, "/pet_app_demo/sche1"))
      {
        dulieu = fbdo.stringData();
        dulieu.remove(0,2);
        dulieu.remove(dulieu.length()-2,2);
        dulieu = "Y" + dulieu;
        Serial.println(dulieu);
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/sche2"))
      {
        dulieu = fbdo.stringData();
        dulieu.remove(0,2);
        dulieu.remove(dulieu.length()-2,2);
        dulieu = "Z" + dulieu;
        Serial.println(dulieu);
      }

      if(Firebase.getString(fbdo, "/pet_app_demo/maxfood"))
      {
        dulieu = fbdo.stringData();
        dulieu.remove(0,2);
        dulieu.remove(dulieu.length()-2,2);
        dulieu = "M" + dulieu;
        Serial.println(dulieu);
      }
      wifi_index = 1;
    } 
    else if (wifi_index == 1) {
      Firebase.deleteNode(fbdo, "/hand_push/push");
      Firebase.deleteNode(fbdo, "/pet_app_demo/sche1");
      Firebase.deleteNode(fbdo, "/pet_app_demo/sche2");
      Firebase.deleteNode(fbdo, "/pet_app_demo/data");
      Firebase.deleteNode(fbdo, "/pet_app_demo/maxfood");
      wifi_index = 0;
    }
    delay(50);
}