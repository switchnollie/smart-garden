#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <EEPROM.h>

const uint8_t MOTOR_PIN = D8;
const uint8_t WATERLEVEL_PIN = D0;
//"FRITZ!Box 7590 SC", "61201344397947594581"
const char *ssid;
const char *passphrase;

Ticker pump_tic;
Ticker pump_tic_intervall;
Ticker water_level_tic;

//Webserver
ESP8266WebServer web_server(80);
DNSServer dns_server;
const byte DNS_PORT = 53;
IPAddress esp_ip(192, 168, 4, 1);

//MQTT
const IPAddress broker_address(192, 168, 178, 110);
WiFiClient client;
PubSubClient mqttClient(client);

//Water level of pump controller
const char *water_level_topic;
//Timestamp when pumpcontroller pumped
const char *pumped_topic;
//Moisture from plant controller
const char *moisture_topic;
//Request pump to pumpcontroller
const char *pump_topic;
//Threshold when pumpcontroller shall pump
const char *moisture_threshhold_topic;
//Intervall when pumpcontroller shall pump
const char *pump_intervall_topic;
//Time duration for pumping
const char *pump_duration_topic;

int moisture_threshhold = 20;   //TODO
int pump_intervall = 600000000; //TODO
int pump_duration = 5000;
// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

void waitforIP();
void WiFiEvent(WiFiEvent_t event);
void pump();
void initializeWebServer();
void publishWaterLevel();
void intializeMQTT();
void set_pump_intervall();
void set_humidty_threshold();
void set_pump_duration();
void mqttCallback(char *topic, byte *payload, unsigned int length);
bool testWifi();

void connect_to_wlan()
{
  EEPROM.begin(512);
  delay(1000);

  //Reading SSID from EEPROM
  Serial.println("Reading EEPROM ssid");
  String ssid;
  for (int i = 0; i < 32; ++i)
  {
    ssid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid);

  //Reading PASS from EEPROM
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Starting web server!");
    initializeWebServer();
  }

  waitforIP();
}

void waitforIP()
{
  Serial.printf("Waiting for IP configuration...");
  while (WiFi.status() != WL_CONNECTED)
  {
    //Allow user to init/change wlan
    web_server.handleClient();
    dns_server.processNextRequest();
  }
}

void reconnect_MQTT()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("connected");
      mqttClient.subscribe(moisture_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void change_wlan()
{
  // if (!web_server.authenticate("user", "password"))
  // {
  //   web_server.requestAuthentication();
  // }
  // else
  // {
    //https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/
    //Write new connection params into EEPROM and connect to WIFI
    String ssid = web_server.arg("ssid").c_str();
    String pass = web_server.arg("pass").c_str();
    Serial.println("Changing WLAN parameters");
    Serial.printf("SSID: %s", ssid.c_str());
    Serial.printf("Password: %s", pass.c_str());
    if (ssid.length() > 0 && pass.length() > 0)
    {
      Serial.println("Clearing eeprom");
      for (int i = 0; i < 96; ++i)
      {
        EEPROM.write(i, 0);
      }
      Serial.println(ssid);
      Serial.println("");
      Serial.println(pass);
      Serial.println("");

      Serial.println("writing eeprom ssid:");
      for (int i = 0; i < ssid.length(); ++i)
      {
        EEPROM.write(i, ssid[i]);
        Serial.print("Wrote: ");
        Serial.println(ssid[i]);
      }
      Serial.println("writing eeprom pass:");
      for (int i = 0; i < pass.length(); ++i)
      {
        EEPROM.write(32 + i, pass[i]);
        Serial.print("Wrote: ");
        Serial.println(pass[i]);
      }
      EEPROM.commit();

      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, pass);

      waitforIP();
      web_server.send(200, "text/plain", "Erfolgreich mit dem WLAN verbunden");
    }
  // }
}

bool testWifi()
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void init_mqtt_topics()
{
  if (!web_server.authenticate("user", "password"))
  {
    web_server.requestAuthentication();
  }
  else
  {
    water_level_topic = web_server.arg("water_level_topic").c_str();
    pumped_topic = web_server.arg("pumped_topic").c_str();
    moisture_topic = web_server.arg("moisture_topic").c_str();
    pump_topic = web_server.arg("pump_topic").c_str();
    moisture_threshhold_topic = web_server.arg("moisture_threshhold_topic").c_str();
    pump_intervall_topic = web_server.arg("pump_intervall_topic").c_str();
    pump_duration_topic = web_server.arg("pump_duration_topic").c_str();
  }
  web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(WATERLEVEL_PIN, INPUT);

  WiFi.onEvent(WiFiEvent);

  //initialize web server first, so the customer is able to initialize/change wlan config
  connect_to_wlan();

  //init MQTT
  mqttClient.setServer(broker_address, 1883);
  mqttClient.setCallback(mqttCallback);

  water_level_tic.attach_ms(6000, publishWaterLevel);
  pump_tic_intervall.attach_ms(pump_intervall, pump);
}

void loop()
{
  web_server.handleClient();
  dns_server.processNextRequest();
  if (!mqttClient.connected())
  {
    reconnect_MQTT();
  }
  mqttClient.loop();
}

void initializeWebServer()
{
  //WIFI ACCESS POINT
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
                    esp_ip,                       //Gateway Adresse
                    IPAddress(255, 255, 255, 0)); //Subnetz-Maske
  WiFi.softAP("ESP");

  web_server.on("/change_wlan", change_wlan);
  web_server.on("/init_mqtt_topics", init_mqtt_topics);

  //redirect
  web_server.onNotFound([]() {
    web_server.sendHeader("Location", "/");
    web_server.send(302, "text/plain", "Pfad nicht verfügbar!");
  });

  web_server.begin();
  dns_server.start(DNS_PORT, "tibs_pump_controller.de", esp_ip);
  Serial.println("Webserver gestartet.");
}

void pumpStart()
{
  Serial.println("Pumping");
  digitalWrite(MOTOR_PIN, HIGH);
  mqttClient.publish(pumped_topic, "pumped");
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

  if (topic == moisture_topic)
  {
    if ((int)payload[0] <= moisture_threshhold)
    {
      pump();
    }
  }
  else if (topic == pump_topic)
  {
    pump();
  }
  else if (topic == moisture_threshhold_topic)
  {
    moisture_threshhold = (int)payload[0];
  }
  else if (topic == pump_intervall_topic)
  {
    pump_tic_intervall.attach_ms((int)payload[0] * 100 * 60 * 60 * 24, pump);
  }
  else if (topic == pump_intervall_topic)
  {
    pump_duration = (int)payload[0];
  }
}

void publishWaterLevel()
{
  int water_level = analogRead(WATERLEVEL_PIN);
  sprintf(messageBuffer, "%d", water_level);
  Serial.print("Publishing message ");
  Serial.println(water_level);
  mqttClient.publish(water_level_topic, messageBuffer);
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case WIFI_EVENT_STAMODE_CONNECTED:
    //Serial.println("Wifi connected!");
    break;
  case WIFI_EVENT_STAMODE_DISCONNECTED:
    //Serial.println("Wifi disconnected!");
    //connect_to_wlan();
    break;
  default:
    break;
  }
}