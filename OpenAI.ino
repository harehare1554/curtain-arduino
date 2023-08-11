#include <Arduino.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <time.h>

#define ntpServer "131.107.1.10"
#define tzOffset 32400

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* serverUrl = "Json URL"; //openとcloseにhh:mm形式でデータが入っていること。
const char* openTime;
const char* closeTime;

bool flag = false; 
const char* opentimez;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, tzOffset, 43200000); //NTP設定

void setup() {

  flag = false;

  pinMode(25,OUTPUT);
  pinMode(26,OUTPUT);
  digitalWrite(25,HIGH);
  digitalWrite(26,HIGH);
  
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");

  configTime(tzOffset, 0, "pool.ntp.org");
  Serial.println("Waiting for NTP time sync...");
  while (!time(nullptr)) {
    delay(100);
  }
  Serial.println("Time synchronized: ");

}

void getDataFromServer() {
  HTTPClient http;
  
  http.begin(serverUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
      DynamicJsonDocument doc(capacity);
      deserializeJson(doc, payload);

      openTime = doc["open"];
      closeTime = doc["close"];

      Serial.print("Value 1: ");
      Serial.println(openTime);
      Serial.print("Value 2: ");
      Serial.println(closeTime);
    }
  } else {
    Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void loop() {
  timeClient.update();

  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  String currentHour0;
  String currentMinute0;

  if (currentHour < 10) {
    currentHour0 = "0" + String(currentHour);
  } else {
    currentHour0 = String(currentHour);
  }
  if (currentMinute < 10) {
    currentMinute0 = "0" + String(currentMinute);
  } else {
    currentMinute0 = String(currentMinute);
  }

  String time = currentHour0 + ":" + currentMinute0;

  Serial.print("Value 3: ");
  Serial.println(time);

  if (time == openTime && flag == false ) {
    digitalWrite(26,LOW);
    delay(9000);
    digitalWrite(26,HIGH);
    Serial.println("It's OpenTime!");
    flag =true;
  }
  if (time == closeTime && flag == true ) {
    digitalWrite(25,LOW);
    delay(9000);
    digitalWrite(25,HIGH);
    Serial.println("It's CloseTime!");
    flag = false;
  }

  delay(1600);
  getDataFromServer();
  delay(400);
}
