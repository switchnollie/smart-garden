#include <Arduino.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <FS.h>
#include <wifi.h>
#include <WiFiClientSecure.h>

const uint8_t MOISTURE_PIN = A0;
const long SENS_INTERVAL = 2000;

const char *ssid;
const char *passphrase;

void init_mqtt_topics(String username, String groupid);

WIFI wifi_controller(init_mqtt_topics);
File root_ca_file;

void intialize_mqtt();
void publish_moisture_level();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

const char *host = "smartgarden.timweise.com";
const uint16_t BROKER_PORT = 8883;
WiFiClientSecure esp_client;
PubSubClient mqtt_client(host, BROKER_PORT, esp_client);
const char *fingerprint = "90:18:60:66:E5:2E:4B:38:09:0D:39:30:9F:64:1E:50:55:11:86:5A";

//Publish moisture
const char *MOISTURE_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int MQTT_INIT_EEPROM_INDEX = 99;

// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

Ticker moisture_level_tic;

const uint8_t MOTOR_PIN = D8;
int motorState = LOW;

void init_mqtt_topics(String username, String groupid)
{

    String prefix = username + "/" + groupid + "/" + ESP.getFlashChipId() + "/";

    String moisture = prefix + "moisture";
    MOISTURE_TOPIC = moisture.c_str();
    Serial.printf("Moisture topic: %s", MOISTURE_TOPIC);

    EEPROM.begin(512);

    //Clear data
    for (int i = 100; i < 160; i++)
    {
        EEPROM.write(i, 0);
    }

    Serial.println("\nWriting moisture topic...");
    for (int i = 0; i < strlen(MOISTURE_TOPIC); ++i)
    {
        EEPROM.write(100 + i, MOISTURE_TOPIC[i]);
        Serial.print(MOISTURE_TOPIC[i]);
    }

    //Write initialized bit
    EEPROM.write(MQTT_INIT_EEPROM_INDEX, 1);
    mqtt_initialized = true;

    EEPROM.commit();
}

void read_mqtt_parameters()
{
    Serial.println("Reading moisture topic...");
    String moisture = "";
    for (int i = 100; i < 160; ++i)
    {
        moisture += char(EEPROM.read(i));
    }
    MOISTURE_TOPIC = moisture.c_str();
    //TODO REMOVE
    MOISTURE_TOPIC = "5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f46c254098c1222a484/moisture";
    Serial.printf("Water level topic: %s\n", MOISTURE_TOPIC);
}

void wait_for_MQTT()
{
    Serial.println("Waiting for MQTT configuration...");
    while (!mqtt_initialized)
    {
        //Allow user to init/change mqtt topics
        wifi_controller.handle_client();
    }
}

void read_mqtt_topics()
{
    EEPROM.begin(512);

    //Check if initialization already done
    int result = EEPROM.read(MQTT_INIT_EEPROM_INDEX);
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
        char mqtt_id[10];
        sprintf(mqtt_id, "%d", ESP.getFlashChipId());
        //TODO REMOVE
        Serial.printf("Client ID: %s", mqtt_id);

        // Attempt to connect
        if (mqtt_client.connect("5f2d2f46c254098c1222a484")) //TODO mqtt_id
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            wifi_controller.handle_client();
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

void load_root_ca()
{
    if (SPIFFS.begin())
    {
        Serial.println("Flash storage succesfully started!");
    }
    else
    {
        Serial.println("Error starting up flash storage");
    }

    root_ca_file = SPIFFS.open("/rootca.pem", "r");

    if (!root_ca_file)
    {
        Serial.println("Error reading RootCA file");
    }
}

void connect_mqtt_client()
{
    esp_client.setFingerprint(fingerprint);

    Serial.print("MQTT: connecting to ");
    Serial.println(host);
    if (!esp_client.connect(host, 8883))
    {
        Serial.println("MQTT TLS connection failed");
        return;
    }

    if (esp_client.verify(fingerprint, host))
    {
        Serial.println("MQTT TLS certificate matches");
    }
    else
    {
        Serial.println("MQTT TLS certificate doesn't match");
    }
}

void init_mqtt()
{
    read_mqtt_topics();
    connect_mqtt_client();
    mqtt_client.setServer(host, BROKER_PORT);
    mqtt_client.setCallback(mqtt_callback);
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(MOISTURE_PIN, INPUT);

    //TODO REMOVE
    delay(5000);
    load_root_ca();
    

    wifi_controller.begin();
    init_mqtt();

    randomSeed(micros());

    moisture_level_tic.attach_ms(SENS_INTERVAL, publish_moisture_level);
}

void loop()
{
    wifi_controller.handle_client();

    if (wifi_controller.status() != WL_CONNECTED)
    {
        wifi_controller.connect_to_wlan();
    }

    // if(!esp_client.connected()){
    //     connect_mqtt_client();
    // }

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
        Serial.printf("Moisture topic: %s\n", MOISTURE_TOPIC);
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
