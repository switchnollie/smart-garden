#include <Arduino.h>
#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("\r\nStarte Access-Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  Serial.println(WiFi.softAPIP());
  delay(1000);
}