#include <Arduino.h>
#include <ESP8266WiFi.h>

const uint8_t MOTOR_PIN = D0;
int motorState = LOW;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("\r\nStarte Access-Point...");
  WiFi.mode(WIFI_AP);
  pinMode(MOTOR_PIN, OUTPUT);
  WiFi.softAP("ESP");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  Serial.println(WiFi.softAPIP());
  digitalWrite(MOTOR_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  Serial.println("Pin is HIGH");
  delay(1000);                   // wait for a second
  digitalWrite(MOTOR_PIN, LOW);  // turn the LED off by making the voltage LOW
  Serial.println("Pin is LOW");
  delay(1000);
}