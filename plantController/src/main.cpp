#include <Arduino.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <FS.h>
#include <wifi.h>

const uint8_t MOISTURE_PIN = A0;
const long SENS_INTERVAL = 2000;

const char *ssid;
const char *passphrase;

WiFiClient client;
const char* API_USER_ENDPOINT = "https://smartgarden.timweise.com/api/user";

void init_mqtt_topics(String username, String groupid);
WIFI wifi_controller(init_mqtt_topics);
File root_ca_file;

void intialize_mqtt();
void publish_moisture_level();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

const IPAddress BROKER_ADDRESS(139, 59, 210, 39);
const uint16_t BROKER_PORT = 8883;
WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

//Publish moisture
const char *MOISTURE_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int MQTT_INIT_EEPROM_INDEX = 99;

int moisture_threshhold = 950; // TODO
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
    for (int i = 100; i < 140; i++)
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
        Serial.printf("Client ID: %s", mqtt_id);
        // Attempt to connect
        if (mqtt_client.connect(mqtt_id))
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
    if (SPIFFS.begin()) {
        Serial.println("Flash storage succesfully started!");
    }
    else {
        Serial.println("Error starting up flash storage");
    }

    root_ca_file = SPIFFS.open("/letsencryptRootCA.pem", "r");

    if (!root_ca_file) {
        Serial.println("Error reading RootCA file");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(MOISTURE_PIN, INPUT);

    load_root_ca();

    wifi_controller.begin();
    read_mqtt_topics();

    //init MQTT
    mqtt_client.setServer(BROKER_ADDRESS, BROKER_PORT);
    mqtt_client.setCallback(mqtt_callback);

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

    //TODO incomment -> dont block for 5s to be able to handle web server
        // if (!mqtt_client.connected())
        // {
        //     reconnect_MQTT();
        // }


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
