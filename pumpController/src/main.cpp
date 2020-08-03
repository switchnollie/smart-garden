#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <string>

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

//Start webserver
const char *webserver_topic;
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
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int mqtt_initialized_eeprom_index = 99;

int moisture_threshhold = 20;   //TODO
int pump_intervall = 600000000; //TODO
int pump_duration = 5000;
// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

void waitforIP();
void WiFiEvent(WiFiEvent_t event);
void pump();
void start_web_server();
void publishWaterLevel();
void intializeMQTT();
void set_pump_intervall();
void set_humidty_threshold();
void set_pump_duration();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
bool testWifi();
String read_wlan_ssid();
String read_wlan_pass();
void change_wlan();
void write_wlan_parameters(String ssid, String pass);
void write_mqtt_parameters();
void read_mqtt_parameters();
int get_string_size(const char *value);

void connect_to_wlan()
{
  EEPROM.begin(512);
  delay(1000);

  String ssid = read_wlan_ssid();
  String pass = read_wlan_pass();

  Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Starting web server!");
    start_web_server();
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

void change_wlan()
{
  //https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/
  //Write new connection params into EEPROM and connect to WIFI
  String ssid = web_server.arg("ssid");
  String pass = web_server.arg("pass");
  Serial.println("Changing WLAN parameters");
  Serial.printf("SSID: %s\n", ssid.c_str());
  Serial.printf("Password: %s\n", pass.c_str());
  if (ssid.length() > 0 && pass.length() > 0)
  {
    write_wlan_parameters(ssid, pass);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    if (testWifi())
    {
      Serial.println("Succesfully Connected!!!");
      return;
    }
    else
    {
      Serial.println("Starting web server!");
      start_web_server();
    }

    waitforIP();
    web_server.send(200, "text/plain", "Erfolgreich mit dem WLAN verbunden");
  }
}

void write_wlan_parameters(String ssid, String pass)
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

  //TODO encrypt password
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
}

String read_wlan_ssid()
{
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
  return ssid;
}

String read_wlan_pass()
{

  //Reading PASS from EEPROM
  Serial.println("Reading EEPROM pass");
  String pass = "";
  for (int i = 32; i < 96; ++i)
  {
    pass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(pass);
  return pass;
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
  water_level_topic = web_server.arg("water_level_topic").c_str();
  Serial.printf("Water level topic: %s", water_level_topic);

  pump_topic = web_server.arg("pump_topic").c_str();
  Serial.printf("Pump topic: %s", pump_topic);

  pumped_topic = web_server.arg("pumped_topic").c_str();
  Serial.printf("Pumped topic: %s", pumped_topic);

  moisture_topic = web_server.arg("moisture_topic").c_str();
  Serial.printf("Moisture topic: %s", moisture_topic);

  moisture_threshhold_topic = web_server.arg("moisture_threshhold_topic").c_str();
  Serial.printf("Moisture threshhold topic: %s", moisture_threshhold_topic);

  pump_intervall_topic = web_server.arg("pump_intervall_topic").c_str();
  Serial.printf("Pump intervall topic: %s", pump_intervall_topic);

  pump_duration_topic = web_server.arg("pump_duration_topic").c_str();
  Serial.printf("Pump duration topic: %s", pump_duration_topic);

  EEPROM.begin(512);
  delay(1000);
  write_mqtt_parameters();

  web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
}
void write_mqtt_parameters()
{
  Serial.println("\nWriting water level topic...");
  std::string water_level = std::string(water_level_topic);
  for (int i = 100; i < 100 + water_level.length(); ++i)
  {
    EEPROM.write(i, water_level[i]);
    Serial.println(water_level[i]);
  }

  Serial.println("\nWriting pumped topic...");
  std::string pumped = std::string(pumped_topic);
  for (int i = 140; i < 140 + pumped.length(); ++i)
  {
    EEPROM.write(i, pumped[i]);
    Serial.println(pumped[i]);
  }

  Serial.println("\nWriting moisture topic...");
  std::string moisture = std::string(moisture_topic);
  for (int i = 180; i < 180 + moisture.length(); ++i)
  {
    EEPROM.write(i, moisture[i]);
    Serial.println(moisture[i]);
  }

  Serial.println("\nWriting pump topic...");
  std::string pump = std::string(pump_topic);
  for (int i = 220; i < 220 + pump.length(); ++i)
  {
    EEPROM.write(i, pump[i]);
    Serial.println(pump[i]);
  }

  Serial.println("\nWriting moisture threshhold topic...");
  std::string moisture_threshhold = std::string(moisture_threshhold_topic);
  for (int i = 260; i < 260 + moisture_threshhold.length(); ++i)
  {
    EEPROM.write(i, moisture_threshhold[i]);
    Serial.println(moisture_threshhold[i]);
  }

  Serial.println("\nWriting pump intervall topic...");
  std::string pump_intervall = std::string(pump_intervall_topic);
  for (int i = 300; i < 300 + pump_intervall.length(); ++i)
  {
    EEPROM.write(i, pump_intervall[i]);
    Serial.print(pump_intervall[i]);
  }

  Serial.println("\nWriting pump duration topic...");
  std::string pump_duration = std::string(pump_duration_topic);
  for (int i = 340; i < 340 + pump_duration.length(); ++i)
  {
    EEPROM.write(i, pump_duration[i]);
    Serial.println(pump_duration[i]);
  }

  //Write initialized bit
  EEPROM.write(mqtt_initialized_eeprom_index, 1);
  mqtt_initialized = true;

  EEPROM.commit();
}

void read_mqtt_parameters()
{
  Serial.println("Reading water level topic...");
  for (int i = 100; i < 140; ++i)
  {
    Serial.println(char(EEPROM.read(i)));
    water_level_topic += char(EEPROM.read(i));
  }
  Serial.printf("Water level topic: %s\n", water_level_topic);

  Serial.println("Reading pumped topic");
  for (int i = 140; i < 180; ++i)
  {
    pumped_topic += char(EEPROM.read(i));
  }
  Serial.printf("Pumped topic: %s\n", pumped_topic);

  Serial.println("Reading moisture topic...");
  for (int i = 180; i < 220; ++i)
  {
    moisture_topic += char(EEPROM.read(i));
  }
  Serial.printf("Moisture topic: %s\n", moisture_topic);

  Serial.println("Reading pump topic...");
  for (int i = 220; i < 260; ++i)
  {
    pump_topic += char(EEPROM.read(i));
  }
  Serial.printf("Pump topic: %s\n", pump_topic);

  Serial.println("Reading moisture threshhold topic...");
  for (int i = 260; i < 300; ++i)
  {
    moisture_threshhold_topic += char(EEPROM.read(i));
  }
  Serial.printf("Moisture threshhold topic: %s\n", moisture_threshhold_topic);

  Serial.println("Reading pump intervall topic...");
  for (int i = 300; i < 340; ++i)
  {
    pump_intervall_topic += char(EEPROM.read(i));
  }
  Serial.printf("Pump intervall topic: %s\n", pump_intervall_topic);

  Serial.println("Reading pump duration topic...");
  for (int i = 340; i < 380; ++i)
  {
    pump_duration_topic += char(EEPROM.read(i));
  }
  Serial.printf("Pump duration topic: %s\n", pump_duration_topic);

  if (water_level_topic != NULL)
  {
    mqtt_initialized = true;
  }
}

void wait_for_MQTT()
{
  Serial.printf("Waiting for MQTT configuration...");
  while (!mqtt_initialized)
  {
    //Allow user to init/change mqtt topics
    web_server.handleClient();
    dns_server.processNextRequest();
  }
}

//Read mqtt topics from EEPROM -> if not provided start webserver
void read_mqtt_topics()
{
  EEPROM.begin(512);
  delay(1000);
  //Check if initialization already done
  int result = EEPROM.read(mqtt_initialized_eeprom_index);
  Serial.printf("result: %d", result);
  if (result != 1)
  {
    mqtt_initialized = false;
  }
  else
  {
    mqtt_initialized = true;
  }

  //TODO remove this
  //mqtt_initialized = false;

  if (mqtt_initialized) 
  {
    read_mqtt_parameters();
  }
  {
    start_web_server();
    wait_for_MQTT();
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

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(WATERLEVEL_PIN, INPUT);

  //TODO can be used to clear eeprom
  // EEPROM.begin(512);
  // delay(1000);
  // for (int i = 90; i < 512; ++i)
  // {
  //   EEPROM.write(i, 0);
  // }

  WiFi.onEvent(WiFiEvent);

  connect_to_wlan();
  read_mqtt_topics();

  //init MQTT
  mqttClient.setServer(broker_address, 1883);
  mqttClient.setCallback(mqtt_callback);

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

void start_web_server()
{
  //WIFI ACCESS POINT
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
                    esp_ip,                       //Gateway Adresse
                    IPAddress(255, 255, 255, 0)); //Subnetz-Maske
  WiFi.softAP("ESP", "ESPPASSWORD");

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

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received MQTT message");
  Serial.print(topic);
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  if (topic == webserver_topic)
  {
    start_web_server();
  }
  else if (topic == moisture_topic)
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
  Serial.printf("Publishing %d to topic %s", water_level, water_level_topic);
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