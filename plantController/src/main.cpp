#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <FS.h>

const uint8_t MOISTURE_PIN = A0;
const long SENS_INTERVAL = 2000;

const char *ssid;
const char *passphrase;

WiFiClient client;
const char* API_USER_ENDPOINT = "https://smartgarden.timweise.com/api/user";

//Webserver
ESP8266WebServer web_server(80);
IPAddress ESP_IP(192, 168, 4, 1);
File wlan_html_file;
File user_html_file;
File   groups_html_file;
File root_ca_file;

void intialize_mqtt();
void publish_moisture_level();
void change_wlan();
void connect_to_wlan();
String read_wlan_ssid();
String read_wlan_pass();
bool test_wifi();
void write_wlan_parameters(String ssid, String pass);
void init_mqtt_topics();
void change_user();
String read_ap_password();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

const IPAddress BROKER_ADDRESS(139, 59, 210, 39);
const uint16_t BROKER_PORT = 8883;
WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);
String user = "";
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

void serve_wlan_html() {
    web_server.streamFile(wlan_html_file, "text/html");
}

void serve_user_html() {
    web_server.streamFile(user_html_file, "text/html");
}

void serve_groups_html() {
    web_server.streamFile(groups_html_file, "text/html");
}

void start_web_server()
{
    Serial.println("Starting web server!");

    //WIFI ACCESS POINT
    WiFi.softAPConfig(ESP_IP,                       //Eigene Adresse
        ESP_IP,                       //Gateway Adresse
        IPAddress(255, 255, 255, 0)); //Subnetz-Maske

    String ap_pass = read_ap_password();
    WiFi.softAP("ESP Pump", ap_pass.c_str());

     //WLAN and AP
    web_server.on("/wlan", serve_wlan_html);
    web_server.on("/change_wlan", change_wlan);

    //User credentials
    web_server.on("/user", serve_user_html);
    web_server.on("/change_user", change_user);

    //Groups
    web_server.on("/groups", serve_groups_html);
    web_server.on("/init_mqtt_topics", init_mqtt_topics);

    //redirect
    web_server.onNotFound([]() {
        web_server.sendHeader("Location", "/wlan");
        web_server.send(302, "text/plain", "Path not available!");
        });

    web_server.begin();
    Serial.println("Webserver started.");
}

void connect_to_wlan()
{
    EEPROM.begin(512);

    String ssid = read_wlan_ssid();
    String pass = read_wlan_pass();

    Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        //Wait till connected
        web_server.handleClient();
    }

    Serial.println("Succesfully Connected!!!");
    return;
}

void change_ap_password() {
    String pass = web_server.arg("pass");

    Serial.println("Writing AP password: ");
    EEPROM.begin(512);
    for (int i = 0; i < pass.length(); ++i)
    {
        EEPROM.write(400 + i, pass[i]);
        Serial.print(pass[i]);
    }

    EEPROM.commit();
}

String read_ap_password() {
    //Reading PASS from EEPROM
    delay(3000);
    Serial.println("Reading AP password: ");
    EEPROM.begin(512);
    String pass = "";
    for (int i = 400; i < 440; ++i)
    {
        pass += char(EEPROM.read(i));
    }
    Serial.print("PASS: ");
    Serial.println(pass);
    if (!pass) {
        return "ESPPASSWORD";
    }
    return pass;
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
        WiFi.begin(ssid, pass);

        if (test_wifi())
        {
            Serial.println("Succesfully Connected!!!");
            web_server.send(200, "text/plane", "Succesfully Connected");
        }
        else {
            web_server.send(400, "text/plain", "Error setting WLAN. Try again!");
        }
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

void send_user_data_to_backend(String username, String pass) {

    if (client.connect(API_USER_ENDPOINT, 80))
    {
        Serial.println("Successfully connected to API Endpoint!");
        client.print(String("GET /") + " HTTP/1.1\r\n" +
            "Host: " + API_USER_ENDPOINT +"?username=" + username + "&pass=" + pass
            + "\r\n" +
            "Connection: close\r\n" +
            "\r\n"
        );
        //Check if data transmitted
        while (client.connected() || client.available())
        {
            if (client.available())
            {
                String line = client.readStringUntil('\n');
                if (!line) {
                    client.stop();
                    Serial.println("\n[Disconnected]");
                    web_server.send(400, "text/plain", "Failed to send user credentials to API Endpoint!");
                }
                else {
                    client.stop();
                    Serial.println("\n[Disconnected]");
                    web_server.send(200, "text/plain", "Successfully send user credentials to API Endpoint!");
                }
            }
        }
    }
    else
    {
        Serial.println("Failed to connect to API Endpoint!");
        client.stop();
        Serial.println("\n[Disconnected]");
        web_server.send(400, "text/plain", "Failed to connect to API Endpoint!");
    }

}

void change_user() {
    String username = web_server.arg("username");
    String pass = web_server.arg("pass");
    send_user_data_to_backend(username, pass);
    user = username;
}

void init_mqtt_topics()
{
    String group = web_server.arg("groupid");
    String moisture = user + "/" + group + "/" + ESP.getFlashChipId() + "/" + "moisture";
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
    EEPROM.write(MQTT_INIT_EEPROM_INDEX, 1);
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

void load_static_files()
{
    if (SPIFFS.begin()) {
        Serial.println("Flash storage succesfully started!");
    }
    else {
        Serial.println("Error starting up flash storage");
    }

    Dir dir = SPIFFS.openDir("/");
    String output = "[";
    while (dir.next()) {
        File entry = dir.openFile("r");
        output += String(entry.name()).substring(1);
        entry.close();
    }
    output +="]";
    Serial.println(output);

    wlan_html_file = SPIFFS.open("/wlan.html", "r");
    user_html_file = SPIFFS.open("/user.html", "r");
    groups_html_file = SPIFFS.open("/groups.html", "r");
    root_ca_file = SPIFFS.open("/letsencryptRootCA.pem", "r");
    if (!wlan_html_file) {
        Serial.println("Error reading wlan.html file");
    }
        if (!user_html_file) {
        Serial.println("Error reading user.html file");
    }
    if (!groups_html_file) {
        Serial.println("Error reading group.html file");
    }
    if (!root_ca_file) {
        Serial.println("Error reading RootCA file");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(MOISTURE_PIN, INPUT);

    load_static_files();

    WiFi.mode(WIFI_AP_STA);
    start_web_server();
    connect_to_wlan();
    read_mqtt_topics();

    //init MQTT
    mqtt_client.setServer(BROKER_ADDRESS, BROKER_PORT);
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
