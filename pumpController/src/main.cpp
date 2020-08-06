#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <string>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

const uint8_t MOTOR_PIN = D8;
const uint8_t WATERLEVEL_PIN = D0;
const char *ssid;
const char *passphrase;
//https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPUpdateServer/examples/WebUpdater/WebUpdater.ino
const char *host = "esp8266-webupdate";

Ticker pump_tic;
Ticker pump_tic_intervall;
Ticker water_level_tic;

//Webserver
ESP8266WebServer web_server(80);
ESP8266HTTPUpdateServer http_updater;
IPAddress esp_ip(192, 168, 4, 1);
File wlan_html_file;
File groups_html_file;
File root_ca_file;

//MQTT
const IPAddress BROKER_ADDRESS(139, 59, 210, 39);
uint16_t BROKER_PORT = 8883;
WiFiClient client;
PubSubClient mqtt_client(client);

//Water level of pump controller
const char *water_level_topic = "";
//Timestamp when pumpcontroller pumped
const char *pumped_topic = "";
//Moisture from plant controller
const char *moisture_topic = "";
//Request pump to pumpcontroller
const char *pump_topic = "";
//Threshold when pumpcontroller shall pump
const char *moisture_threshhold_topic = "";
//Intervall when pumpcontroller shall pump
const char *pump_intervall_topic = "";
//Time duration for pumping
const char *pump_duration_topic = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int mqtt_initialized_eeprom_index = 99;

int moisture_threshhold = 20;   //TODO
int pump_intervall = 600000000; //TODO
int pump_duration = 5000;
// messages are 10 Bit decimals -> max. 4 characters + \0 needed
#define MSG_BUFFER_SIZE 5
char messageBuffer[MSG_BUFFER_SIZE];

void pump();
void start_web_server();
void publishWaterLevel();
void intialize_mqtt();
void set_pump_intervall();
void set_humidty_threshold();
void set_pump_duration();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
bool test_wifi();
String read_wlan_ssid();
String read_wlan_pass();
void change_wlan();
void write_wlan_parameters(String ssid, String pass);
void write_mqtt_parameters();
void read_mqtt_parameters();
void serve_groups_html();

void connect_to_wlan()
{
    EEPROM.begin(512);

    String ssid = read_wlan_ssid();
    String pass = read_wlan_pass();

    Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());
    if (test_wifi())
    {
        Serial.println("Succesfully Connected!!!");
        return;
    }

    Serial.printf("Waiting for IP configuration...");
    while (WiFi.status() != WL_CONNECTED)
    {
        //Wait till connected
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

void init_mqtt_topics()
{
    String username = web_server.arg("username");
    String group = web_server.arg("groupid");
    String prefix = username + "/" + group + "/" + ESP.getFlashChipId() + "/";

    String water_level = prefix + "water_level";
    water_level_topic = water_level.c_str();
    Serial.printf("Water level topic: %s", water_level_topic);

    String pump = prefix + "pump";
    pump_topic = pump.c_str();
    Serial.printf("Pump topic: %s", pump_topic);

    String pumped = prefix + "pumped";
    pumped_topic = pump.c_str();
    Serial.printf("Pumped topic: %s", pumped_topic);

    String moisture = prefix + "moisture";
    moisture_topic = moisture.c_str();
    Serial.printf("Moisture topic: %s", moisture_topic);

    String moisture_threshhold = prefix + "moisture_threshhold";
    moisture_threshhold_topic = moisture_threshhold.c_str();
    Serial.printf("Moisture threshhold topic: %s", moisture_threshhold_topic);

    String pump_intervall = prefix + "pump_intervall";
    pump_intervall_topic = pump_intervall.c_str();
    Serial.printf("Pump intervall topic: %s", pump_intervall_topic);

    String pump_duration = prefix + "pump_duration";
    pump_duration_topic = pump_duration.c_str();
    Serial.printf("Pump duration topic: %s", pump_duration_topic);

    EEPROM.begin(512);

    write_mqtt_parameters();

    web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
}
void write_mqtt_parameters()
{
    Serial.println("\nWriting water level topic...");
    for (int i = 0; i < strlen(water_level_topic); ++i)
    {
        EEPROM.write(100 + i, water_level_topic[i]);
        Serial.print(water_level_topic[i]);
    }

    Serial.println("\nWriting pumped topic...");
    for (int i = 0; i < strlen(pumped_topic); ++i)
    {
        EEPROM.write(140 + i, pumped_topic[i]);
        Serial.print(pumped_topic[i]);
    }

    Serial.println("\nWriting moisture topic...");
    for (int i = 0; i < strlen(moisture_topic); ++i)
    {
        EEPROM.write(180 + i, moisture_topic[i]);
        Serial.print(moisture_topic[i]);
    }

    Serial.println("\nWriting pump topic...");
    for (int i = 0; i < strlen(pump_topic); ++i)
    {
        EEPROM.write(220 + i, pump_topic[i]);
        Serial.print(pump_topic[i]);
    }

    Serial.println("\nWriting moisture threshhold topic...");
    for (int i = 0; i < strlen(moisture_threshhold_topic); ++i)
    {
        EEPROM.write(260 + i, moisture_threshhold_topic[i]);
        Serial.print(moisture_threshhold_topic[i]);
    }

    Serial.println("\nWriting pump intervall topic...");
    for (int i = 0; i < strlen(pump_intervall_topic); ++i)
    {
        EEPROM.write(300 + i, pump_intervall_topic[i]);
        Serial.print(pump_intervall_topic[i]);
    }

    Serial.println("\nWriting pump duration topic...");
    for (int i = 0; i < strlen(pump_duration_topic); ++i)
    {
        EEPROM.write(340 + i, pump_duration_topic[i]);
        Serial.print(pump_duration_topic[i]);
    }

    //Write initialized bit
    EEPROM.write(mqtt_initialized_eeprom_index, 1);
    mqtt_initialized = true;

    EEPROM.commit();
}

void read_mqtt_parameters()
{
    Serial.println("Reading water level topic...");
    String water_level = "";
    for (int i = 100; i < 140; ++i)
    {
        water_level += char(EEPROM.read(i));
    }
    water_level_topic = water_level.c_str();
    Serial.printf("Water level topic: %s\n", water_level_topic);

    Serial.println("Reading pumped topic...");
    String pumped = "";
    for (int i = 140; i < 180; ++i)
    {
        pumped += char(EEPROM.read(i));
    }
    pumped_topic = pumped.c_str();
    Serial.printf("Pumped topic: %s\n", pumped_topic);

    Serial.println("Reading moisture topic...");
    String moisture = "";
    for (int i = 180; i < 220; ++i)
    {
        moisture += char(EEPROM.read(i));
    }
    moisture_topic = moisture.c_str();
    Serial.printf("Moisture topic: %s\n", moisture_topic);

    Serial.println("Reading pump topic...");
    String pump = "";
    for (int i = 220; i < 260; ++i)
    {
        pump += char(EEPROM.read(i));
    }
    pump_topic = pump.c_str();
    Serial.printf("Pump topic: %s\n", pump_topic);

    Serial.println("Reading moisture threshhold topic...");
    String moisture_threshhold = "";
    for (int i = 260; i < 300; ++i)
    {
        moisture_threshhold += char(EEPROM.read(i));
    }
    moisture_threshhold_topic = moisture_threshhold.c_str();
    Serial.printf("Moisture threshhold topic: %s\n", moisture_threshhold_topic);

    Serial.println("Reading pump intervall topic...");
    String pump_intervall = "";
    for (int i = 300; i < 340; ++i)
    {
        pump_intervall += char(EEPROM.read(i));
    }
    pump_intervall_topic = pump_intervall.c_str();
    Serial.printf("Pump intervall topic: %s\n", pump_intervall_topic);

    Serial.println("Reading pump duration topic...");
    String pump_duration = "";
    for (int i = 340; i < 380; ++i)
    {
        pump_duration += char(EEPROM.read(i));
    }
    pump_duration_topic = pump_duration.c_str();
    Serial.printf("Pump duration topic: %s\n", pump_duration_topic);
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

//Read mqtt topics from EEPROM -> if not provided start webserver
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
            mqtt_client.subscribe(moisture_topic);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            web_server.handleClient();
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

    Serial.println("EEPROM cleared");
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
    groups_html_file = SPIFFS.open("/groups.html", "r");
    root_ca_file = SPIFFS.open("/letsencryptRootCA.pem.html", "r");
    if (!wlan_html_file) {
        Serial.println("Fehler beim Einlesen der wlan.html Datei");
    }
    if (!groups_html_file) {
        Serial.println("Fehler beim Einlesen der group.html Datei");
    }
    if (!root_ca_file) {
        Serial.println("Fehler beim Einlesen der RootCA Datei");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(WATERLEVEL_PIN, INPUT);

    load_static_files();
    //clear_eeprom();

    WiFi.mode(WIFI_AP_STA);
    start_web_server();
    connect_to_wlan();
    read_mqtt_topics();

    //init MQTT
    mqtt_client.setServer(BROKER_ADDRESS, BROKER_PORT);
    mqtt_client.setCallback(mqtt_callback);

    water_level_tic.attach_ms(6000, publishWaterLevel);
    pump_tic_intervall.attach_ms(pump_intervall, pump);
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
    MDNS.update();
}

void serve_wlan_html() {
    web_server.streamFile(wlan_html_file, "text/html");
}

void serve_groups_html() {
    web_server.streamFile(groups_html_file, "text/html");
}

void start_web_server()
{
    //WIFI ACCESS POINT
    WiFi.softAPConfig(esp_ip,                       //Eigene Adresse
        esp_ip,                       //Gateway Adresse
        IPAddress(255, 255, 255, 0)); //Subnetz-Maske
    WiFi.softAP("ESP Pump", "ESPPASSWORD");

    web_server.on("/wlan", serve_wlan_html);
    web_server.on("/change_wlan", change_wlan);
    web_server.on("/groups", serve_groups_html);
    web_server.on("/init_mqtt_topics", init_mqtt_topics);

    //redirect
    web_server.onNotFound([]() {
        web_server.sendHeader("Location", "/");
        web_server.send(302, "text/plain", "Path not available!");
        });

    //MDNS.begin(host);

    //http_updater.setup(&web_server);

    web_server.begin();
    Serial.println("Webserver started.");

    //MDNS.addService("http", "tcp", 80);
    //Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void pumpStart()
{
    Serial.println("Pumping");
    digitalWrite(MOTOR_PIN, HIGH);
    mqtt_client.publish(pumped_topic, "pumped");
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
    if (topic == moisture_topic)
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
    if (mqtt_client.connected())
    {
        int water_level = analogRead(WATERLEVEL_PIN);
        sprintf(messageBuffer, "%d", water_level);
        Serial.printf("Publishing %d to topic %s\n", water_level, water_level_topic);
        mqtt_client.publish(water_level_topic, messageBuffer);
    }
}