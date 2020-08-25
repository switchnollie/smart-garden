#include <Arduino.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <string>
#include <FS.h>
#include <wifi.h>

const uint8_t MOTOR_PIN = D8;
const uint8_t WATERLEVEL_PIN = D0;
const char *ssid;
const char *passphrase;
//https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPUpdateServer/examples/WebUpdater/WebUpdater.ino

WiFiClient client;
const char *API_USER_ENDPOINT = "https://smartgarden.timweise.com/api/user";

Ticker pump_tic_start;
Ticker pump_tic_stop;
Ticker water_level_tic;

//WIFI
void init_mqtt_topics(String username, String group_id);
WIFI wifi_controller(init_mqtt_topics);
File root_ca_file;

//MQTT
const char *host = "smartgarden.timweise.com";
const uint16_t port = 8883;
WiFiClientSecure esp_client;
PubSubClient mqtt_client(host, port, esp_client);
const char *fingerprint = "90:18:60:66:E5:2E:4B:38:09:0D:39:30:9F:64:1E:50:55:11:86:5A";

//Water level of pump controller
const char *WATER_LEVEL_TOPIC = "";
//Request pump to pumpcontroller
const char *PUMP_TOPIC = "";
//Time duration for pumping
const char *PUMP_DURATION_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int MQTT_INIT_EEPROM_INDEX = 99;

int pump_duration = 10000;
// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

void pump();
void publishWaterLevel();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void read_mqtt_parameters();

void read_mqtt_parameters()
{
    Serial.println("Reading water level topic...");
    String water_level = "";
    for (int i = 100; i < 160; ++i)
    {
        water_level += char(EEPROM.read(i));
    }
    WATER_LEVEL_TOPIC = water_level.c_str();
    Serial.printf("Water level topic: %s\n", WATER_LEVEL_TOPIC);

    Serial.println("Reading pump topic...");
    String pump = "";
    for (int i = 160; i < 220; ++i)
    {
        pump += char(EEPROM.read(i));
    }
    PUMP_TOPIC = pump.c_str();
    //TODO REMOVE
    PUMP_TOPIC = "5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f515e9536fb08962ba5/pump";
    Serial.printf("Pump topic: %s\n", PUMP_TOPIC);

    Serial.println("Reading pump duration topic...");
    String pump_duration = "";
    for (int i = 220; i < 280; ++i)
    {
        pump_duration += char(EEPROM.read(i));
    }
    PUMP_DURATION_TOPIC = pump_duration.c_str();
    Serial.printf("Pump duration topic: %s\n", PUMP_DURATION_TOPIC);
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

//Read mqtt topics from EEPROM -> if not provided start webserver
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
        // Attempt to connect
        char mqtt_id[10];
        sprintf(mqtt_id, "%d", ESP.getFlashChipId());
        Serial.printf("Client ID: %s", mqtt_id);
        if (mqtt_client.connect("5f2d2f515e9536fb08962ba5")) //TODO mqtt_id
        {
            Serial.println("connected");
            mqtt_client.subscribe(PUMP_TOPIC);
            Serial.printf("Subscribed to %s", PUMP_TOPIC);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            wifi_controller.handle_client();
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
    EEPROM.commit();

    Serial.println("EEPROM cleared");
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

    root_ca_file = SPIFFS.open("/letsencryptRootCA.pem", "r");

    if (!root_ca_file)
    {
        Serial.println("Error reading RootCA file");
    }
}

void connect_mqtt_client()
{
    esp_client.setFingerprint(fingerprint);

    Serial.print("connecting to ");
    Serial.println(host);
    if (!esp_client.connect(host, 8883))
    {
        Serial.println("connection failed");
        return;
    }

    if (esp_client.verify(fingerprint, host))
    {
        Serial.println("certificate matches");
    }
    else
    {
        Serial.println("certificate doesn't match");
    }
}

void init_mqtt()
{
    read_mqtt_topics();
    connect_mqtt_client();
    mqtt_client.setServer(host, port);
    mqtt_client.setCallback(mqtt_callback);
}


void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    clear_eeprom(); //TODO REMOVE

    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(WATERLEVEL_PIN, INPUT);

    load_root_ca();

    wifi_controller.begin();
    read_mqtt_topics();

    init_mqtt();
}

void loop()
{
    wifi_controller.handle_client();

    if (wifi_controller.status() != WL_CONNECTED)
    {
        wifi_controller.connect_to_wlan();
    }
    if (!mqtt_client.connected())
    {
        reconnect_MQTT();
    }

    mqtt_client.loop();
}
void write_mqtt_parameters()
{
    //Clear data
    EEPROM.begin(512);
    for (int i = 100; i < 380; i++)
    {
        EEPROM.write(i, 0);
    }

    Serial.println("\nWriting water level topic...");
    for (int i = 0; i < strlen(WATER_LEVEL_TOPIC); ++i)
    {
        EEPROM.write(100 + i, WATER_LEVEL_TOPIC[i]);
        Serial.print(WATER_LEVEL_TOPIC[i]);
    }

    Serial.println("\nWriting pump topic...");
    for (int i = 0; i < strlen(PUMP_TOPIC); ++i)
    {
        EEPROM.write(160 + i, PUMP_TOPIC[i]);
        Serial.print(PUMP_TOPIC[i]);
    }

    Serial.println("\nWriting pump duration topic...");
    for (int i = 0; i < strlen(PUMP_DURATION_TOPIC); ++i)
    {
        EEPROM.write(220 + i, PUMP_DURATION_TOPIC[i]);
        Serial.print(PUMP_DURATION_TOPIC[i]);
    }

    //Write initialized bit
    EEPROM.write(MQTT_INIT_EEPROM_INDEX, 1);
    mqtt_initialized = true;

    EEPROM.commit();
}

void init_mqtt_topics(String username, String groupid)
{
    String prefix = username + "/" + groupid + "/" + ESP.getFlashChipId() + "/";

    String water_level = prefix + "water_level";
    WATER_LEVEL_TOPIC = water_level.c_str();
    Serial.printf("Water level topic: %s", WATER_LEVEL_TOPIC);

    String pump = prefix + "pump";
    PUMP_TOPIC = pump.c_str();
    Serial.printf("Pump topic: %s", PUMP_TOPIC);

    String pump_duration = prefix + "pump_duration";
    PUMP_DURATION_TOPIC = pump_duration.c_str();
    Serial.printf("Pump duration topic: %s", PUMP_DURATION_TOPIC);

    EEPROM.begin(512);

    write_mqtt_parameters();
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

//Creates two function calls; one for immediatly starting the pump
//Other for stopping the pump after certain duration from user
void pump()
{
    Serial.printf("Starting to pump for %d seconds", pump_duration);
    pump_tic_start.once_ms(0, pumpStart);
    pump_tic_stop.once_ms(pump_duration, pumpStop);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println("Received MQTT message");
    Serial.printf("Topic: %s", topic);

    pump();

    //TODO didnt match
    // if (topic == PUMP_TOPIC)
    // {
    //     pump();
    // }
    // else if (topic == PUMP_DURATION_TOPIC)
    // {
    //     pump_duration = (int)payload[0];
    // }
}

void publishWaterLevel()
{
    //TODO incomment
    // if (mqtt_client.connected())
    // {
    //     int water_level = analogRead(WATERLEVEL_PIN);
    //     sprintf(messageBuffer, "%d", water_level);
    //     Serial.printf("Publishing %d to topic %s\n", water_level, WATER_LEVEL_TOPIC);
    //     mqtt_client.publish(WATER_LEVEL_TOPIC, messageBuffer);
    // }
}