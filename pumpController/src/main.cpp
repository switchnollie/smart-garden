#include <Arduino.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <string>
#include <FS.h>
#include <wifi.h>

void pump();
void publishWaterLevel();
void connect_mqtt_client();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void read_mqtt_parameters();
void init_mqtt_topics(String username, String group_id);
String send_user_data(DynamicJsonDocument, String);

const uint8_t MOTOR_PIN = D8;
const uint8_t WATERLEVEL_PIN = A0;

//https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPUpdateServer/examples/WebUpdater/WebUpdater.ino

Ticker pump_tic_start;
Ticker pump_tic_stop;
Ticker water_level_tic;
const long SENS_INTERVAL = 10000;

//WIFI
WIFI wifi_controller(init_mqtt_topics, send_user_data);
File root_ca_file;

//MQTT
const char *host = "smartgarden.timweise.com";
const uint16_t port = 8883;
WiFiClientSecure esp_client;
PubSubClient mqtt_client(host, port, esp_client);
const char *fingerprint = "90:18:60:66:E5:2E:4B:38:09:0D:39:30:9F:64:1E:50:55:11:86:5A";
String authorization_code = "";

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

String send_user_data(DynamicJsonDocument doc, String url)
{
    String response = "";

    if (esp_client.connected())
    {
        Serial.println("Stopping current connection");
        esp_client.stop();
    }

    Serial.print("Connecting to ");
    Serial.println(host);

    if (esp_client.connect(host, 443)) //Soft WDT Reset with >5s
    {
        Serial.println("Posting data to " + (String)host + url + "...");
        serializeJsonPretty(doc, Serial);

        esp_client.println("POST " + url + " HTTP/1.1");
        esp_client.println(String("Host: ") + host);
        esp_client.println("Content-Type: application/json");
        if (authorization_code.length() > 0)
        {
            esp_client.println("Authorization: " + authorization_code);
        }
        esp_client.println("Connection: close");
        esp_client.print("Content-Length: ");
        esp_client.println(measureJsonPretty(doc));
        esp_client.println();

        // Write JSON document
        serializeJsonPretty(doc, esp_client);

        if (esp_client.println() == 0)
        {
            Serial.println("\nFailed to send request");
            return response;
        }

        Serial.println("\nRequest sent.");

        //https://arduinojson.org/v6/example/http-client/
        // Check HTTP status
        char status[32] = {0};

        while (!esp_client.available())
            delay(1);

        Serial.println("Reading response...");

        esp_client.readBytesUntil('\r', status, sizeof(status));

        if (strcmp(status, "HTTP/1.1 200 OK") != 0 && strcmp(status, "HTTP/1.1 201 Created") != 0)
        {
            Serial.print(F("Unexpected response: "));
            Serial.println(status);
            esp_client.stop();
            return response;
        }
        else
        {
            Serial.print("Status Code: ");
            Serial.println(status);
        }

        // Skip HTTP headers
        char endOfHeaders[] = "\r\n\r\n";
        if (!esp_client.find(endOfHeaders))
        {
            Serial.println(F("Invalid response"));
            esp_client.stop();
            return response;
        }

        // Allocate the JSON document
        // Use arduinojson.org/v6/assistant to compute the capacity.
        size_t capacity = 2000;

        DynamicJsonDocument doc(capacity);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, esp_client);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return response;
        }

        // Extract values
        Serial.println(F("Response:"));

        if (url == "/api/user/register" || url == "/api/user/login")
        {
            authorization_code = doc["token"].as<char *>();
            response = doc["user"]["_id"].as<char *>();

            if (authorization_code.length() > 0)
            {
                Serial.println("Authorization Code received.");
            }
        }
        else
        {
            response = doc["_id"].as<char *>();
        }

        // Disconnect
        esp_client.stop();
        return response;
    }
    else
    {
        Serial.println("Failed to connect to API Endpoint!");
        esp_client.stop();
        Serial.println("\n[Disconnected]");
        return "";
    }
}

void read_mqtt_parameters()
{
    Serial.println("Reading water level topic...");
    String water_level = "";
    for (int i = 140; i < 220; ++i)
    {
        water_level += char(EEPROM.read(i));
    }
    WATER_LEVEL_TOPIC = strdup(water_level.c_str());
    Serial.printf("Water level topic: %s\n", WATER_LEVEL_TOPIC);

    Serial.println("Reading pump topic...");
    String pump = "";
    for (int i = 220; i < 300; ++i)
    {
        pump += char(EEPROM.read(i));
    }
    PUMP_TOPIC = strdup(pump.c_str());
    Serial.printf("Pump topic: %s\n", PUMP_TOPIC);

    Serial.println("Reading pump duration topic...");
    String pump_duration = "";
    for (int i = 300; i < 380; ++i)
    {
        pump_duration += char(EEPROM.read(i));
    }
    PUMP_DURATION_TOPIC = strdup(pump_duration.c_str());
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
    //TLS Connection with wifi client
    if (!esp_client.connected())
    {
        connect_mqtt_client();
    }

    // Loop until we're reconnected
    while (!mqtt_client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        char mqtt_id[10];
        sprintf(mqtt_id, "%d", ESP.getChipId());
        Serial.printf("Client ID: %s", mqtt_id);
        if (mqtt_client.connect(mqtt_id))
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
    for (int i = 97; i < 512; i++)
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

void init_esp_client()
{
    esp_client.allowSelfSignedCerts();
    esp_client.loadCACert(root_ca_file);
    esp_client.setFingerprint(fingerprint);
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

    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(WATERLEVEL_PIN, INPUT);

    load_root_ca();

    init_esp_client();
    wifi_controller.begin();
    init_mqtt();

    water_level_tic.attach_ms(SENS_INTERVAL, publishWaterLevel);
}

void loop()
{
    if (wifi_controller.status() != WL_CONNECTED)
    {
        wifi_controller.connect_to_wlan();
    }

    wifi_controller.handle_client();

    if (!mqtt_client.connected())
    {
        reconnect_MQTT();
    }

    mqtt_client.loop();
}
void write_mqtt_parameters(String water_level, String pump, String pump_duration)
{
    //Clear data
    EEPROM.begin(512);
    for (int i = 100; i < 380; i++)
    {
        EEPROM.write(i, 0);
    }

    Serial.println("\nWriting water level topic...");
    for (int i = 0; i < water_level.length(); ++i)
    {
        EEPROM.write(140 + i, water_level[i]);
        Serial.print(water_level[i]);
    }

    Serial.println("\nWriting pump topic...");
    for (int i = 0; i < pump.length(); ++i)
    {
        EEPROM.write(220 + i, pump[i]);
        Serial.print(pump[i]);
    }

    Serial.println("\nWriting pump duration topic...");
    for (int i = 0; i < pump_duration.length(); ++i)
    {
        EEPROM.write(300 + i, pump_duration[i]);
        Serial.print(pump_duration[i]);
    }

    //Write initialized bit
    EEPROM.write(MQTT_INIT_EEPROM_INDEX, 1);
    mqtt_initialized = true;

    EEPROM.commit();
}

void init_mqtt_topics(String user_id, String groupid)
{
    String prefix = user_id + "/" + groupid + "/" + ESP.getChipId() + "/";

    String water_level = prefix + "water_level";
    WATER_LEVEL_TOPIC = strdup(water_level.c_str());
    Serial.printf("Water level topic: %s", WATER_LEVEL_TOPIC);

    String pump = prefix + "pump";
    PUMP_TOPIC = strdup(pump.c_str());
    Serial.printf("Pump topic: %s", PUMP_TOPIC);

    String pump_duration = prefix + "pump_duration";
    PUMP_DURATION_TOPIC = strdup(pump_duration.c_str());
    Serial.printf("Pump duration topic: %s", PUMP_DURATION_TOPIC);

    EEPROM.begin(512);

    write_mqtt_parameters(water_level, pump, pump_duration);
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

    if (strcmp(topic, PUMP_TOPIC) == 0)
    {
        pump();
    }
    else if (strcmp(topic, PUMP_DURATION_TOPIC) == 0)
    {
        pump_duration = (int)payload[0];
    }
}

void publishWaterLevel()
{
    if (mqtt_client.connected())
    {
        int water_level = analogRead(WATERLEVEL_PIN);
        sprintf(messageBuffer, "%d", water_level);
        Serial.printf("Publishing %d to topic %s\n", water_level, WATER_LEVEL_TOPIC);
        mqtt_client.publish(WATER_LEVEL_TOPIC, messageBuffer);
    }
}