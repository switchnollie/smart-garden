#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <EEPROM.h>

const uint8_t MOISTURE_PIN = A0;
const long SENS_INTERVAL = 2000;

const char *ssid;
const char *passphrase;

//Webserver
ESP8266WebServer web_server(80);
IPAddress esp_ip(192, 168, 4, 1);

void intialize_mqtt();
void publish_moisture_level();
void change_wlan();
void connect_to_wlan();
String read_wlan_ssid();
String read_wlan_pass();
bool test_wifi();
void write_wlan_parameters(String ssid, String pass);
void init_mqtt_topics();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

const IPAddress broker_address(192, 168, 0, 143);
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
//Publish moisture
const char *MOISTURE_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int mqtt_initialized_eeprom_index = 99;

int moisture_threshhold = 950; // TODO
// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

Ticker moisture_level_tic;

const uint8_t MOTOR_PIN = D8;
int motorState = LOW;

void start_web_server()
{
  delay(2000);
  Serial.println("Starting web server!");

  //WIFI ACCESS POINT
  WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
                    esp_ip,                       //Gateway Adresse
                    IPAddress(255, 255, 255, 0)); //Subnetz-Maske
  WiFi.softAP("ESP Plant", "ESPPASSWORD");

  web_server.on("/change_wlan", change_wlan);
  web_server.on("/init_mqtt_topics", init_mqtt_topics);

  //redirect
  web_server.onNotFound([]() {
    web_server.sendHeader("Location", "/");
    web_server.send(302, "text/plain", "Pfad nicht verfügbar!");
  });

  web_server.begin();
  Serial.println("Webserver gestartet.");
}

void connect_to_wlan()
{
  EEPROM.begin(512);

  String ssid = read_wlan_ssid();
  String pass = read_wlan_pass();

  Serial.printf("Trying to connect to %s", (char *)ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  if (test_wifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }

  Serial.printf("Waiting for IP configuration...");
  while (WiFi.status() != WL_CONNECTED)
  {
    //Allow user to init/change wlan
    web_server.handleClient();
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

    if (test_wifi())
    {
      Serial.println("Succesfully Connected!!!");
      return;
    }

    Serial.printf("Waiting for IP configuration...");
    while (WiFi.status() != WL_CONNECTED)
    {
      //Allow user to init/change wlan
      web_server.handleClient();
    }

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
    Serial.print(ssid[i]);
  }
  Serial.println("\nwriting eeprom pass:");
  for (int i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(32 + i, pass[i]);
    Serial.print(pass[i]);
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

bool test_wifi()
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
  Serial.println("Connect timed out");
  return false;
}

void init_mqtt_topics()
{
  String username = web_server.arg("username");
  String group = web_server.arg("group");
  String moisture = username + "/" + group + "/" + ESP.getFlashChipId() + "/" + "moisture";
  MOISTURE_TOPIC = moisture.c_str();
  Serial.printf("Moisture topic: %s", MOISTURE_TOPIC);

  EEPROM.begin(512);

  Serial.println("\nWriting moisture topic...");
  for (int i = 0; i < strlen(MOISTURE_TOPIC); ++i)
  {
    EEPROM.write(100 + i, MOISTURE_TOPIC[i]);
    Serial.print(MOISTURE_TOPIC[i]);
  }

  //Write initialized bit
  EEPROM.write(mqtt_initialized_eeprom_index, 1);
  mqtt_initialized = true;

  EEPROM.commit();

  web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
}

void read_mqtt_parameters()
{
  Serial.println("Reading water level topic...");
  String moisture = "";
  for (int i = 100; i < 140; ++i)
  {
    moisture += char(EEPROM.read(i));
  }
  MOISTURE_TOPIC = moisture.c_str();
  Serial.printf("Water level topic: %s\n", MOISTURE_TOPIC);
}

void wait_for_MQTT()
{
  Serial.printf("Waiting for MQTT configuration...");
  while (!mqtt_initialized)
  {
    //Allow user to init/change mqtt topics
    web_server.handleClient();
  }
}

void read_mqtt_topics()
{
  EEPROM.begin(512);

  //Check if initialization already done
  int result = EEPROM.read(mqtt_initialized_eeprom_index);
  if (result != 1)
  {
    mqtt_initialized = false;
  }
  else
  {
    mqtt_initialized = true;
  }

  if (mqtt_initialized)
  {
    read_mqtt_parameters();
  }
  else
  {
    wait_for_MQTT();
  }
}

void reconnect_MQTT()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish("outTopic", "hello world");
      // ... and resubscribe
      mqtt_client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void clear_eeprom()
{
  // Reset EEPROM bytes to '0' for the length of the data structure
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0);
  }
  delay(200);
  EEPROM.commit();
  EEPROM.end();
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(MOISTURE_PIN, INPUT);

  WiFi.mode(WIFI_AP_STA);
  start_web_server();
  connect_to_wlan();
  read_mqtt_topics();

  //init MQTT
  mqtt_client.setServer(broker_address, 1883);
  mqtt_client.setCallback(mqtt_callback);

  randomSeed(micros());

  moisture_level_tic.attach_ms(SENS_INTERVAL, publish_moisture_level);
}

void loop()
{
  web_server.handleClient();

    if (WiFi.status() != WL_CONNECTED)
  {
    connect_to_wlan();
  }

  if (!mqtt_client.connected())
  {
    reconnect_MQTT();
  }


  mqtt_client.loop();
}

void publish_moisture_level()
{
  if (mqtt_client.connected())
  {
    int moistureLevel = analogRead(MOISTURE_PIN);
    sprintf(messageBuffer, "%d", moistureLevel);
    Serial.print("Publishing message ");
    Serial.println(messageBuffer);
    mqtt_client.publish(MOISTURE_TOPIC, messageBuffer);
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received MQTT message");
  Serial.print(topic);
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
}
