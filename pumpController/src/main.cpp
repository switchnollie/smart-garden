#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>

const uint8_t MOTOR_PIN = D8;
const uint8_t WATERLEVEL_PIN = D0;

Ticker pump_tic;
Ticker pump_tic_intervall;
Ticker water_level_tic;

//Webserver
ESP8266WebServer web_server(80);
DNSServer dns_server;
const byte DNS_PORT = 53;
IPAddress esp_ip(192, 168, 4, 1);

//MQTT
const char *MQTT_BROKER = "TODO IP";
WiFiClient client;
PubSubClient mqttClient(client);
const char *WATER_LEVEL_TOPIC = "sensor/waterlevel";
const char *PUMPED_TOPIC = "sensor/pumped";
const char *HUMIDITY_TOPIC = "sensor/humidity";
int humidity_threshhold = 20;   //TODO
int pump_intervall = 600000000; //TODO
int pump_duration = 5000;

void WiFiEvent(WiFiEvent_t event);
void pump();
void initializeWebServer();
void publishWaterLevel();
void intializeMQTT();
void set_pump_intervall();
void set_humidty_threshold();
void set_pump_duration();

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
    Serial.println("Connected to FRITZ!Box 7590 SC");
  }
}

void connect_to_wlan()
{
  Serial.println("Connecting to WIFI...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("FRITZ!Box 7590 SC", "61201344397947594581"); //TODO this should be generalized //pw:
  waitforIP();
}

void change_wlan()
{
  web_server.send(200, "text/plain", "OK");
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(web_server.arg("ssid").c_str(), web_server.arg("pass").c_str());
  waitforIP();
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(WATERLEVEL_PIN, INPUT);

  //Connect to WIFI
  connect_to_wlan();

  initializeWebServer();
  intializeMQTT();

  water_level_tic.attach_ms(6000, publishWaterLevel);
  pump_tic_intervall.attach_ms(pump_intervall, pump);
}

void loop()
{
  web_server.handleClient();
  dns_server.processNextRequest();
}

void set_humidty_threshold()
{
  humidity_threshhold = web_server.arg("threshhold").toInt();
}

void set_pump_intervall()
{
  pump_tic_intervall.attach_ms(web_server.arg("intervall").toInt() * 100 * 60 * 60 * 24, pump);
}

void set_pump_duration()
{
  pump_duration = web_server.arg("duration").toInt();
}

void initializeWebServer()
{
  //WIFI ACCESS POINT
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
                    esp_ip,                       //Gateway Adresse
                    IPAddress(255, 255, 255, 0)); //Subnetz-Maske
  WiFi.softAP("ESP");

  web_server.on("/pump", pump);
  web_server.on("/change_wlan", change_wlan);
  web_server.on("/humidity_threshhold", set_humidty_threshold);
  web_server.on("/pump_intervall", set_pump_intervall);
  web_server.on("/pump_duration", set_pump_duration);

  //redirect
  web_server.onNotFound([]() {
    web_server.sendHeader("Location", "/");
    web_server.send(302, "text/plain", "Pfad nicht verfügbar!");
  });

  web_server.begin();
  dns_server.start(DNS_PORT, "tibs_pump_controller.de", esp_ip);
}

void pumpStart()
{
  Serial.println("Pumping");
  digitalWrite(MOTOR_PIN, HIGH);
  mqttClient.publish(PUMPED_TOPIC, "pumped");
}

void pumpStop()
{
  Serial.println("Stopped pumping");
  digitalWrite(MOTOR_PIN, LOW);
}

//Creates to function calls; one for immediatly starting the pump
//Other for stopping the pump after certain duration from user
void pump()
{
  Serial.printf("Starting to pump for %d seconds", pump_duration);
  pump_tic.once_ms(0, pumpStart);
  pump_tic.once_ms(pump_duration * 1000, pumpStop);
  web_server.send(200);
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received MQTT message");
  Serial.print(topic);
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  if (topic == HUMIDITY_TOPIC)
  {
    if ((int)payload[0] <= humidity_threshhold)
    {
      pump();
    }
  }
}

void intializeMQTT()
{
  mqttClient.setServer(MQTT_BROKER, 1883); //TODO PORT
  mqttClient.subscribe(HUMIDITY_TOPIC);
  mqttClient.setCallback(mqttCallback);
}

void publishWaterLevel()
{
  Serial.println(analogRead(WATERLEVEL_PIN));
  //mqttClient.publish(WATER_LEVEL_TOPIC, (char*)analogRead(WATERLEVEL_PIN));
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.println("Wifi connected!");
    break;
  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("Wifi disconnected!");
    connect_to_wlan();
    break;
  default:
    break;
  }
}