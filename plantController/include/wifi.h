#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <functional>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

class WIFI
{
public:
    WIFI(std::function<void(String, String)> callback_mqtt, std::function<String(char[], String)> callback_send_user_data);
    void begin();
    int status();
    void handle_client();
    void connect_to_wlan();

private:
    void start_web_server();
    String read_wlan_ssid();
    String read_wlan_pass();
    void change_ap_password();
    String read_ap_password();
    void write_wlan_parameters(String ssid, String pass);
    void change_wlan();
    bool test_wifi();

    ESP8266WebServer web_server;
    ESP8266HTTPUpdateServer http_updater;
    const char *dns_host = "esp8266";
    String user_id = "";

    std::function<void(String, String)> init_mqtt_topics_callback_implementation;
    std::function<String(char[], String)> send_user_data_callback_implementation;
    void send_user_data_to_backend();
    void send_water_group_to_backend();

    const char *host = "smartgarden.timweise.com";
};

WIFI::WIFI(std::function<void(String, String)> callback_mqtt, std::function<String(char[], String)> callback_send_user_data)
{
    init_mqtt_topics_callback_implementation = callback_mqtt;
    send_user_data_callback_implementation = callback_send_user_data;
}

void WIFI::begin()
{
    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_AP_STA);
    start_web_server();
    connect_to_wlan();
}

int WIFI::status()
{
    return WiFi.status();
}

void WIFI::handle_client()
{
    web_server.handleClient();
    MDNS.update();
}

void WIFI::start_web_server()
{
    //WIFI ACCESS POINT
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),    //Eigene Adresse
                      IPAddress(192, 168, 4, 1),    //Gateway Adresse
                      IPAddress(255, 255, 255, 0)); //Subnetz-Maske

    String ap_pass = read_ap_password();
    WiFi.softAP("ESP Plant", ap_pass.c_str());

    //WLAN and AP
    web_server.serveStatic("/init", SPIFFS, "/wlan.html");
    web_server.on("/changewlan", [this]() {
        change_wlan();
    });

    //User credentials
    web_server.serveStatic("/user", SPIFFS, "/user.html");
    web_server.on("/changeuser", [this]() {
        send_user_data_to_backend();
    });

    //Groups
    web_server.serveStatic("/groups", SPIFFS, "/groups.html");
    web_server.on("/initmqtttopics", [this]() {
        send_water_group_to_backend();
    });

    //redirect
    web_server.onNotFound([this]() { //this cant be used in lambda function
        web_server.sendHeader("Location", "/init");
        web_server.send(302, "text/plain", "Path not available!");
    });

    if (!MDNS.begin(dns_host))
    {
        Serial.println("Fehler beim Aufsetzen des DNS!");
    }
    http_updater.setup(&web_server);

    web_server.begin(80);

    MDNS.addService("http", "tcp", 80);
    Serial.printf("Webserver gestarted! Öffne http://%s.local/<Pfad> zur Bedienung!\n", dns_host);
}

String WIFI::read_wlan_ssid()
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

String WIFI::read_wlan_pass()
{
    //Reading PASS from EEPROM
    Serial.println("Reading WLAN password...");
    String pass = "";
    for (int i = 32; i < 96; ++i)
    {
        pass += char(EEPROM.read(i));
    }
    return pass;
}

void WIFI::connect_to_wlan()
{
    EEPROM.begin(512);

    String ssid = read_wlan_ssid();
    String pass = read_wlan_pass();

    if (strlen((char *)ssid.c_str()) > 0)
    {
        Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());
    }
    else
    {
        Serial.println("Waiting for WLAN initialization!");
    }
    while (WiFi.status() != WL_CONNECTED)
    {
        //Wait till connected
        handle_client();
    }

    Serial.println("Succesfully Connected!!!");
    return;
}

void WIFI::change_wlan()
{

    change_ap_password();

    //https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/
    //Write new connection params into EEPROM and connect to WIFI
    String ssid = web_server.arg("ssid");
    String pass = web_server.arg("pass");
    Serial.println("Changing WLAN parameters");
    Serial.printf("SSID: %s\n", ssid.c_str());
    if (ssid.length() > 0 && pass.length() > 0)
    {
        write_wlan_parameters(ssid, pass);
        WiFi.begin(ssid, pass);

        if (test_wifi())
        {
            Serial.println("Succesfully Connected! Sending response...");
            web_server.send(200, "text/plane", "Succesfully Connected");
        }
        else
        {
            Serial.println("Error setting WLAN");
            web_server.send(400, "text/plane", "Error connecting to WLAN");
        }
    }
}

void WIFI::write_wlan_parameters(String ssid, String pass)
{
    Serial.println("Clearing eeprom");
    for (int i = 0; i < 96; ++i)
    {
        EEPROM.write(i, 0);
    }
    Serial.println(ssid);

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
        Serial.print("*");
    }
    EEPROM.commit();
}

void WIFI::change_ap_password()
{
    String pass = web_server.arg("apPass");

    Serial.println("Writing AP password: ");
    EEPROM.begin(512);
    for (int i = 0; i < pass.length(); ++i)
    {
        EEPROM.write(300 + i, pass[i]);
        Serial.print(pass[i]);
    }

    EEPROM.commit();
}

String WIFI::read_ap_password()
{
    //Reading PASS from EEPROM
    Serial.println("Reading AP password: ");
    EEPROM.begin(512);
    String pass = "";
    for (int i = 300; i < 340; ++i)
    {
        pass += char(EEPROM.read(i));
    }
    if (strlen((char *)pass.c_str()) == 0)
    {
        return "ESPPASSWORD";
    }
    return pass;
}

bool WIFI::test_wifi()
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

void WIFI::send_user_data_to_backend()
{
    Serial.println("Sending user data to backend");

    DynamicJsonDocument JSONencoder(1024);
    JSONencoder["username"] = web_server.arg("userId");
    JSONencoder["password"] = web_server.arg("pass");

    //Print json object to string
    char JSONmessageBuffer[300];
    serializeJsonPretty(JSONencoder, JSONmessageBuffer);

    user_id = send_user_data_callback_implementation(JSONmessageBuffer, "/api/user/register");
    Serial.println("User ID: " + user_id);
    if (user_id.length() > 0)
    {
        web_server.send(200, "text/plain", "Successfully sent user data!");
    }
    else
    {
        web_server.send(400, "text/plain", "Failed to connect to API Endpoint!");
    }
}

void WIFI::send_water_group_to_backend()
{
    Serial.println("Sending group data to backend");
    DynamicJsonDocument JSONencoder(1024);
    DynamicJsonDocument JSONdevice(1024);

    JSONencoder["displayName"] = web_server.arg("groupId");
    JSONencoder["ownedBy"] = user_id;

    JsonArray devices = JSONencoder.createNestedArray("devices");
    JSONdevice["_id"] = ESP.getFlashChipId();
    JSONdevice["displayName"] = "Plant";
    JSONdevice["type"] = "moisture";

    devices.add(JSONdevice);

    //Print json object to string
    char JSONmessageBuffer[300];
    serializeJsonPretty(JSONencoder, JSONmessageBuffer);

    String group_id = send_user_data_callback_implementation(JSONmessageBuffer, "/api/wateringroup");

    Serial.println("Group ID: " + group_id);

    if (user_id.length() > 0)
    {
        web_server.send(200, "text/plain", "Successfully sent user data!");
        init_mqtt_topics_callback_implementation(user_id, group_id);
    }
    else
    {
        web_server.send(400, "text/plain", "Failed to set group!");
    }
}