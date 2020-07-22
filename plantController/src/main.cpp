#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

const uint8_t MOISTURE_PIN = A0;
const long SENS_INTERVAL = 2000;
void intializeMQTT();
void publishMoistureLevel();

const IPAddress broker_address(192,168,0,143);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
const char *HUMIDITY_TOPIC = "sensor/moisture";
int humidity_threshhold = 950;  // TODO
// messages are 10 Bit decimals -> max. 4 characters needed
#define MSG_BUFFER_SIZE 4
char messageBuffer[MSG_BUFFER_SIZE];


Ticker moisture_level_tic;

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

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
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
             "<pwd>");  // TODO this should be generalized //pw:
  waitforIP();
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  intializeMQTT();
  moisture_level_tic.attach_ms(SENS_INTERVAL, publishMoistureLevel);
}

void loop() { 
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  // publishMoistureLevel();
}

void publishMoistureLevel() {
  // char* moistureLevel = (char *)analogRead(MOISTURE_PIN);
  Serial.println("Tick");
  long randomNumber = random(0, 1023);
  ltoa(randomNumber, messageBuffer, 10);
  Serial.println(messageBuffer);
  mqttClient.publish(HUMIDITY_TOPIC, messageBuffer);
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received MQTT message");
  Serial.print(topic);
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void intializeMQTT() {
  mqttClient.setServer(broker_address, 1883);
  mqttClient.subscribe(HUMIDITY_TOPIC);
  mqttClient.setCallback(mqttCallback);
}

