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
    WIFI(std::function<void(String, String)> callback);
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

    const char *host = "smartgarden.timweise.com";
};

WIFI::WIFI(std::function<void(String, String)> callback)
{
    init_mqtt_topics_callback_implementation = callback;
}

void WIFI::begin()
{
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
        user_id = web_server.arg("user-id");
    });

    //Groups
    web_server.serveStatic("/groups", SPIFFS, "/groups.html");
    web_server.on("/initmqtttopics", [this]() {
        init_mqtt_topics_callback_implementation(user_id, web_server.arg("groupid"));
        web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
    });

    //redirect
    web_server.onNotFound([this]() { //this cant be used in lambda function
        web_server.sendHeader("Location", "/init");
        web_server.send(302, "text/plain", "Path not available!");
    });

    MDNS.begin(dns_host);
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

    Serial.printf("Trying to connec to %s", (char *)ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());

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
    if (!pass)
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

