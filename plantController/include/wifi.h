#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <functional>

//typedef void (*init_mqtt_topics_callback)(String username, String group_id);

class WIFI {
public:
    WIFI(std::function<void(String, String)> callback);
    void begin();
    int status();
    void handle_client();
    void connect_to_wlan();

private:
    void load_html_files();
    void start_web_server();
    String read_wlan_ssid();
    String read_wlan_pass();
    void change_ap_password();
    String read_ap_password();
    void write_wlan_parameters(String ssid, String pass);
    void change_wlan();
    bool test_wifi();
    void send_user_data_to_backend(String username, String pass);
    void change_user();

    ESP8266WebServer web_server;
    File wlan_html_file;
    File user_html_file;
    File groups_html_file;
    String user = "";
    WiFiClient client;
    std::function<void(String, String)> init_mqtt_topics_callback_implementation;

    const char* API_USER_ENDPOINT = "https://smartgarden.timweise.com/api/user";
};

WIFI::WIFI(std::function<void(String, String)> callback) {
    init_mqtt_topics_callback_implementation = callback;
}

void WIFI::begin() {
    web_server.begin(80);
    load_html_files();
    WiFi.mode(WIFI_AP_STA);
    start_web_server();
    connect_to_wlan();
}

int WIFI::status() {
    return WiFi.status();
}

void WIFI::handle_client() {
    web_server.handleClient();
}

void WIFI::load_html_files()
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

    if (!wlan_html_file) {
        Serial.println("Error reading wlan.html file");
    }
    if (!user_html_file) {
        Serial.println("Error reading user.html file");
    }
    if (!groups_html_file) {
        Serial.println("Error reading group.html file");
    }
}

void WIFI::start_web_server()
{
    //WIFI ACCESS POINT
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),                       //Eigene Adresse
        IPAddress(192, 168, 4, 1),                       //Gateway Adresse
        IPAddress(255, 255, 255, 0)); //Subnetz-Maske

    String ap_pass = read_ap_password();
    WiFi.softAP("ESP Plant", ap_pass.c_str());

    //WLAN and AP
    web_server.on("/wlan", [this]() {
        web_server.streamFile(wlan_html_file, "text/html");
        });
    web_server.on("/change_wlan", [this]() {
        change_wlan();
        });

    //User credentials
    web_server.on("/user", [this]() {
        web_server.streamFile(user_html_file, "text/html");
        });
    web_server.on("/change_user", [this]() {
        change_user();
        });

    //Groups
    web_server.on("/groups", [this]() {
        web_server.streamFile(groups_html_file, "text/html");
        });
    web_server.on("/init_mqtt_topics", [this]() {
        init_mqtt_topics_callback_implementation(user, web_server.arg("groupid"));
        web_server.send(200, "text/plain", "MQTT Topics erfolgreich gesetzt");
        });

    //redirect
    web_server.onNotFound([this]() { //this cant be used in lambda function
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

String  WIFI::read_wlan_ssid() {
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

String  WIFI::read_wlan_pass() {
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

void WIFI::connect_to_wlan() {
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


void WIFI::change_wlan()
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

void WIFI::write_wlan_parameters(String ssid, String pass)
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

void WIFI::change_ap_password() {
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

String WIFI::read_ap_password() {
    //Reading PASS from EEPROM
    Serial.println("Reading AP password: ");
    EEPROM.begin(512);
    String pass = "";
    for (int i = 300; i < 340; ++i)
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

void WIFI::send_user_data_to_backend(String username, String pass) {

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

void WIFI::change_user() {
    String username = web_server.arg("username");
    String pass = web_server.arg("pass");
    send_user_data_to_backend(username, pass);
    user = username;
}
