#include <Arduino.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <FS.h>
#include <wifi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

void init_mqtt_topics(String username, String groupid);
String send_user_data(DynamicJsonDocument, String);
void connect_mqtt_client();
void intialize_mqtt();
void publish_moisture_level();

const uint8_t MOISTURE_PIN = A0;
const uint8_t MOTOR_PIN = D8;

Ticker moisture_level_tic;
const long SENS_INTERVAL = 5000;

//WIFI
WIFI wifi_controller(init_mqtt_topics, send_user_data);

//MQTT
const char *host = "smartgarden.timweise.com";
const uint16_t BROKER_PORT = 8883;
WiFiClientSecure esp_client;
PubSubClient mqtt_client(host, BROKER_PORT, esp_client);
const char *MOISTURE_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int MQTT_INIT_EEPROM_INDEX = 99;

//Authentification
const char *fingerprint = "90:18:60:66:E5:2E:4B:38:09:0D:39:30:9F:64:1E:50:55:11:86:5A";
File root_ca_file;
String authorization_code = "";

// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

/*
Sends user data to api/user/register or api/user/login user in initialization routine
Reads token and user id from response
*/
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

            if(authorization_code.length() > 0){
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

void init_mqtt_topics(String user_id, String groupid)
{
    
    String prefix = user_id + "/" + groupid + "/" + ESP.getChipId() + "/";
    String moisture = prefix + "moisture";

    MOISTURE_TOPIC = strdup(moisture.c_str());
    Serial.printf("Moisture topic: %s", MOISTURE_TOPIC);

    EEPROM.begin(512);
    //Clear data
    for (int i = 140; i < 220; i++)
    {
        EEPROM.write(i, 0);
    }

    Serial.println("\nWriting moisture topic...");
    for (int i = 0; i < strlen(MOISTURE_TOPIC); ++i)
    {
        EEPROM.write(140 + i, MOISTURE_TOPIC[i]);
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
    for (int i = 140; i < 220; ++i)
    {
        moisture += char(EEPROM.read(i));
    }
    MOISTURE_TOPIC = strdup(moisture.c_str());

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
    //Secure Connection with wifi client
    if (!esp_client.connected())
    {
        connect_mqtt_client();
    }

    // Loop until we're reconnected
    while (!mqtt_client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        char mqtt_id[10];
        sprintf(mqtt_id, "%d", ESP.getChipId());

        Serial.printf("Client ID: %s", mqtt_id);

        // Attempt to connect
        if (mqtt_client.connect(mqtt_id)) 
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

    root_ca_file = SPIFFS.open("/rootca.pem", "r");

    if (!root_ca_file)
    {
        Serial.println("Error reading RootCA file");
    }
}

void connect_mqtt_client()
{
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
    mqtt_client.setServer(host, BROKER_PORT);
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(MOISTURE_PIN, INPUT);

    //TODO REMOVE
    clear_eeprom();

    load_root_ca();
    init_esp_client();

    wifi_controller.begin();
    init_mqtt();

    randomSeed(micros());

    moisture_level_tic.attach_ms(SENS_INTERVAL, publish_moisture_level);
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

void publish_moisture_level()
{
    if (mqtt_client.connected())
    {
        int moistureLevel = analogRead(MOISTURE_PIN);
        sprintf(messageBuffer, "%d", moistureLevel);
        Serial.printf("Publishing %d to topic %s\n", moistureLevel, MOISTURE_TOPIC);
        mqtt_client.publish(MOISTURE_TOPIC, messageBuffer);
    }
}

