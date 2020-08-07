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

WiFiClient client;
const char* API_USER_ENDPOINT = "https://smartgarden.timweise.com/api/user";

Ticker pump_tic;
Ticker pump_tic_intervall;
Ticker water_level_tic;

//Webserver
ESP8266WebServer web_server(80);
ESP8266HTTPUpdateServer http_updater;
const IPAddress ESP_IP(192, 168, 4, 1);
File wlan_html_file;
File user_html_file;
File groups_html_file;
File root_ca_file;

//MQTT
const IPAddress BROKER_ADDRESS(139, 59, 210, 39);
const uint16_t BROKER_PORT = 8883;
WiFiClient esp_mqtt_client;
PubSubClient mqtt_client(esp_mqtt_client);

String user = "";

//Water level of pump controller
const char *WATER_LEVEL_TOPIC = "";
//Timestamp when pumpcontroller pumped
const char *PUMPED_TOPIC = "";
//Moisture from plant controller
const char *MOISTURE_TOPIC = "";
//Request pump to pumpcontroller
const char *PUMP_TOPIC = "";
//Threshold when pumpcontroller shall pump
const char *MOISTURE_THRESHHOLD_TOPIC = "";
//Intervall when pumpcontroller shall pump
const char *PUMP_INTERVALL_TOPIC = "";
//Time duration for pumping
const char *PUMP_DURATION_TOPIC = "";
//Flag to indicate initialization of topics
bool mqtt_initialized = false;
const int MQTT_INIT_EEPROM_INDEX = 99;

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

    while (WiFi.status() != WL_CONNECTED)
    {
        //Wait till connected
        web_server.handleClient();
    }

    Serial.println("Succesfully Connected!!!");
    return;
}


void change_ap_password() {
    String pass = web_server.arg("ap-pass");

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

    change_ap_password();

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
    Serial.print("Pass: ");
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
    String prefix = user + "/" + group + "/" + ESP.getFlashChipId() + "/";

    String water_level = prefix + "water_level";
    WATER_LEVEL_TOPIC = water_level.c_str();
    Serial.printf("Water level topic: %s", WATER_LEVEL_TOPIC);

    String pump = prefix + "pump";
    PUMP_TOPIC = pump.c_str();
    Serial.printf("Pump topic: %s", PUMP_TOPIC);

    String pumped = prefix + "pumped";
    PUMPED_TOPIC = pump.c_str();
    Serial.printf("Pumped topic: %s", PUMPED_TOPIC);

    String moisture = prefix + "moisture";
    MOISTURE_TOPIC = moisture.c_str();
    Serial.printf("Moisture topic: %s", MOISTURE_TOPIC);

    String moisture_threshhold = prefix + "moisture_threshhold";
    MOISTURE_THRESHHOLD_TOPIC = moisture_threshhold.c_str();
    Serial.printf("Moisture threshhold topic: %s", MOISTURE_THRESHHOLD_TOPIC);

    String pump_intervall = prefix + "pump_intervall";
    PUMP_INTERVALL_TOPIC = pump_intervall.c_str();
    Serial.printf("Pump intervall topic: %s", PUMP_INTERVALL_TOPIC);

    String pump_duration = prefix + "pump_duration";
    PUMP_DURATION_TOPIC = pump_duration.c_str();
    Serial.printf("Pump duration topic: %s", PUMP_DURATION_TOPIC);

    EEPROM.begin(512);

    write_mqtt_parameters();

    web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
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

    Serial.println("\nWriting pumped topic...");
    for (int i = 0; i < strlen(PUMPED_TOPIC); ++i)
    {
        EEPROM.write(140 + i, PUMPED_TOPIC[i]);
        Serial.print(PUMPED_TOPIC[i]);
    }

    Serial.println("\nWriting moisture topic...");
    for (int i = 0; i < strlen(MOISTURE_TOPIC); ++i)
    {
        EEPROM.write(180 + i, MOISTURE_TOPIC[i]);
        Serial.print(MOISTURE_TOPIC[i]);
    }

    Serial.println("\nWriting pump topic...");
    for (int i = 0; i < strlen(PUMP_TOPIC); ++i)
    {
        EEPROM.write(220 + i, PUMP_TOPIC[i]);
        Serial.print(PUMP_TOPIC[i]);
    }

    Serial.println("\nWriting moisture threshhold topic...");
    for (int i = 0; i < strlen(MOISTURE_THRESHHOLD_TOPIC); ++i)
    {
        EEPROM.write(260 + i, MOISTURE_THRESHHOLD_TOPIC[i]);
        Serial.print(MOISTURE_THRESHHOLD_TOPIC[i]);
    }

    Serial.println("\nWriting pump intervall topic...");
    for (int i = 0; i < strlen(PUMP_INTERVALL_TOPIC); ++i)
    {
        EEPROM.write(300 + i, PUMP_INTERVALL_TOPIC[i]);
        Serial.print(PUMP_INTERVALL_TOPIC[i]);
    }

    Serial.println("\nWriting pump duration topic...");
    for (int i = 0; i < strlen(PUMP_DURATION_TOPIC); ++i)
    {
        EEPROM.write(340 + i, PUMP_DURATION_TOPIC[i]);
        Serial.print(PUMP_DURATION_TOPIC[i]);
    }

    //Write initialized bit
    EEPROM.write(MQTT_INIT_EEPROM_INDEX, 1);
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
    WATER_LEVEL_TOPIC = water_level.c_str();
    Serial.printf("Water level topic: %s\n", WATER_LEVEL_TOPIC);

    Serial.println("Reading pumped topic...");
    String pumped = "";
    for (int i = 140; i < 180; ++i)
    {
        pumped += char(EEPROM.read(i));
    }
    PUMPED_TOPIC = pumped.c_str();
    Serial.printf("Pumped topic: %s\n", PUMPED_TOPIC);

    Serial.println("Reading moisture topic...");
    String moisture = "";
    for (int i = 180; i < 220; ++i)
    {
        moisture += char(EEPROM.read(i));
    }
    MOISTURE_TOPIC = moisture.c_str();
    Serial.printf("Moisture topic: %s\n", MOISTURE_TOPIC);

    Serial.println("Reading pump topic...");
    String pump = "";
    for (int i = 220; i < 260; ++i)
    {
        pump += char(EEPROM.read(i));
    }
    PUMP_TOPIC = pump.c_str();
    Serial.printf("Pump topic: %s\n", PUMP_TOPIC);

    Serial.println("Reading moisture threshhold topic...");
    String moisture_threshhold = "";
    for (int i = 260; i < 300; ++i)
    {
        moisture_threshhold += char(EEPROM.read(i));
    }
    MOISTURE_THRESHHOLD_TOPIC = moisture_threshhold.c_str();
    Serial.printf("Moisture threshhold topic: %s\n", MOISTURE_THRESHHOLD_TOPIC);

    Serial.println("Reading pump intervall topic...");
    String pump_intervall = "";
    for (int i = 300; i < 340; ++i)
    {
        pump_intervall += char(EEPROM.read(i));
    }
    PUMP_INTERVALL_TOPIC = pump_intervall.c_str();
    Serial.printf("Pump intervall topic: %s\n", PUMP_INTERVALL_TOPIC);

    Serial.println("Reading pump duration topic...");
    String pump_duration = "";
    for (int i = 340; i < 380; ++i)
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
        web_server.handleClient();
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
        if (mqtt_client.connect(mqtt_id))
        {
            Serial.println("connected");
            mqtt_client.subscribe(MOISTURE_TOPIC);
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
    EEPROM.commit();

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
    // if (!mqtt_client.connected())
    // {
    //     reconnect_MQTT();
    // }

    //mqtt_client.loop();
    MDNS.update();
}

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
    mqtt_client.publish(PUMPED_TOPIC, "pumped");
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
    if (topic == MOISTURE_TOPIC)
    {
        if ((int)payload[0] <= moisture_threshhold)
        {
            pump();
        }
    }
    else if (topic == PUMP_TOPIC)
    {
        pump();
    }
    else if (topic == MOISTURE_THRESHHOLD_TOPIC)
    {
        moisture_threshhold = (int)payload[0];
    }
    else if (topic == PUMP_INTERVALL_TOPIC)
    {
        pump_tic_intervall.attach_ms((int)payload[0] * 100 * 60 * 60 * 24, pump);
    }
    else if (topic == PUMP_INTERVALL_TOPIC)
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