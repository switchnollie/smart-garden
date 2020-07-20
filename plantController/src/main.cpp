#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

const uint8_t MOISTURE_PIN = A0;
void intializeMQTT();
void publishMoistureLevel();

Ticker moisture_level_tic;

DNSServer dns_server;
const byte DNS_PORT = 53;
IPAddress esp_ip(192, 168, 4, 1);

// MQTT
const char *MQTT_BROKER = "TODO IP";
WiFiClient client;
PubSubClient mqttClient(client);
const char *HUMIDITY_TOPIC = "sensor/moisture";
int humidity_threshhold = 950;  // TODO

void waitforIP() {
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 100) {
    Serial.println("Trying to connect to WLAN ...");
    delay(100);
  }

  if (i >= 100) {
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println("Lunger31");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(MOISTURE_PIN, INPUT);
  // WiFi Connection
  Serial.println("Connecting to WIFI...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("Lunger31",
             "Hungerbuehl31!");  // TODO this should be generalized //pw:
  waitforIP();
  intializeMQTT();
  moisture_level_tic.attach_ms(60000, publishMoistureLevel);
}

void loop() { dns_server.processNextRequest(); }

void publishMoistureLevel() {
  mqttClient.publish(HUMIDITY_TOPIC, (char *)analogRead(MOISTURE_PIN));
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received MQTT message");
  Serial.print(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void intializeMQTT() {
  mqttClient.setServer(MQTT_BROKER, 1883);  // TODO PORT
  mqttClient.subscribe(HUMIDITY_TOPIC);
  mqttClient.setCallback(mqttCallback);
}
