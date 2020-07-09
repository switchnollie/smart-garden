#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <Ticker.h>

const uint8_t MOTOR_PIN = D8;
const uint8_t HUMIDITY_PIN = D0;

Ticker pumpTic;

ESP8266WebServer webServer(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress esp_ip(192, 168, 4, 1);
void WiFiEvent(WiFiEvent_t event);

void pump();
void initializeWebServer();

void waitforIP()
{
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 100)
  {
    Serial.println("Trying to connect to WLAN ...");
    delay(100);
  }

  if (i >= 100)
  {
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println("FRITZ!Box 7590 SC");
  }
}

void change_wlan()
{
  webServer.send(200, "text/plain", "OK");
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(webServer.arg("ssid").c_str(), webServer.arg("pass").c_str());
  waitforIP();
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(HUMIDITY_PIN, INPUT);

  //Connect to WIFI

  Serial.println("Connecting to WIFI...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("FRITZ!Box 7590 SC", "61201344397947594581"); //TODO this should be generalized //pw:
  waitforIP();

  initializeWebServer();
}

void loop()
{
  webServer.handleClient();
  dnsServer.processNextRequest();
}

void initializeWebServer()
{
  //WIFI ACCESS POINT
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
                    esp_ip,                       //Gateway Adresse
                    IPAddress(255, 255, 255, 0)); //Subnetz-Maske
  WiFi.softAP("ESP");

  webServer.on("/pump", pump);
  webServer.on("/change_wlan", change_wlan);
  //redirect
  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "/");
    webServer.send(302, "text/plain", "Pfad nicht verfügbar!");
  });

  webServer.begin();
  dnsServer.start(DNS_PORT, "tibs_pump_controller.de", esp_ip);
}

void pumpStart()
{
  Serial.println("Pumping");
  digitalWrite(MOTOR_PIN, HIGH);
}

void pumpStop()
{
  Serial.println("Stopped pumping");
  digitalWrite(MOTOR_PIN, LOW);
}

//Creates to function calls; one for immediatly starting the pump
//Other for stopping the pump after certain intervall from user
void pump()
{
  int intervall = webServer.arg("intervall").toInt();
  //Serial.printf("Starting to pump for %d seconds", intervall);
  Serial.printf("Starting to pump for %d seconds", intervall);
  pumpTic.once_ms(0, pumpStart);
  pumpTic.once_ms(intervall*1000, pumpStop);
  webServer.send(200);
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.println("Wifi connected!");
    break;
  default:
    break;
  }
}