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
    WIFI(std::function<void(String, String)> callback, std::function<String(DynamicJsonDocument, String)> callback_send_user_data);
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
    std::function<String(DynamicJsonDocument, String)> send_user_data_callback_implementation;
    void send_user_data_to_backend();
    void send_water_group_to_backend();

    const char *host = "smartgarden.timweise.com";
};

WIFI::WIFI(std::function<void(String, String)> callback, std::function<String(DynamicJsonDocument, String)> callback_send_user_data)
{
    init_mqtt_topics_callback_implementation = callback;
    send_user_data_callback_implementation = callback_send_user_data;
}

void WIFI::begin()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoConnect(false);
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
    WiFi.softAP("ESP Pump", ap_pass.c_str());

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
    Serial.println("Reading EEPROM pass");
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
        web_server.handleClient();
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
            web_server.send(400, "text/plain", "Error setting WLAN. Try again!");
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
    Serial.println("");

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
    String pass = web_server.arg("ap-pass");

    Serial.println("Writing AP password: ");
    EEPROM.begin(512);
    for (int i = 0; i < pass.length(); ++i)
    {
        EEPROM.write(100 + i, pass[i]);
        Serial.print("*");
    }

    EEPROM.commit();
}

String WIFI::read_ap_password()
{
    //Reading PASS from EEPROM
    Serial.println("Reading AP password...");
    EEPROM.begin(512);
    String pass = "";
    for (int i = 100; i < 140; ++i)
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
    Serial.println("Waiting for Wifi to connect...");
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
    Serial.println("Connect timed out...");
    return false;
}

void WIFI::send_user_data_to_backend()
{
    Serial.println("Sending user data to backend");

    DynamicJsonDocument doc(1024);
    doc["username"] = web_server.arg("userId");
    doc["password"] = web_server.arg("pass");

    String url;
    String register_user = web_server.arg("register");

    if (web_server.arg("register") == "false")
    { 
        url = "/api/user/login";
    }
    else
    {
        url = "/api/user/register";
    }

    user_id = send_user_data_callback_implementation(doc, url);
    Serial.println("User ID: " + user_id);
    if (user_id.length() > 0)
    {
        web_server.send(200, "text/plain", "Successfully sent user data!");
    }
    else
    {
        web_server.send(400, "text/plain", "Error processing user data!");
    }
}

void WIFI::send_water_group_to_backend()
{
    Serial.println("Sending group data to backend");
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument doc_waterlevel(1024);
    DynamicJsonDocument doc_pump(1024);

    doc["displayName"] = web_server.arg("groupId");
    doc["ownedBy"] = user_id;

    JsonArray devices = doc.createNestedArray("devices");

    doc_waterlevel["_id"] = (String)ESP.getChipId() + "-w";
    doc_waterlevel["displayName"] = "Water level Sens";
    doc_waterlevel["type"] = "waterlevel";

    doc_pump["_id"] = ESP.getChipId();
    doc_pump["displayName"] = "Pump";
    doc_pump["type"] = "pump";

    devices.add(doc_waterlevel);
    devices.add(doc_pump);

    String group_id = send_user_data_callback_implementation(doc, "/api/wateringgroup");

    Serial.println("Group ID: " + group_id);

    if (group_id.length() > 0)
    {
        web_server.send(200, "text/plain", "Successfully sent user data!");
        init_mqtt_topics_callback_implementation(user_id, group_id);
    }
    else
    {
        web_server.send(400, "text/plain", "Failed to set group!");
    }
}