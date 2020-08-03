# Server

## Vorgehensweise
1) Mit dem einrichtenden Gerät in das ESP WLAN (SSID = "ESP", PW = "ESPPASSWORD")
2) Wlan ändern durch aufrufen des Links unter <Wlan ändern:> mit SSID und Passwort
3) Erneut mit dem ESP WLAN verbinden
4) MQTT Topics setzen durch Aufrufen des Links unter <MQTT Topics ändern:>
   
## Config
IP: 192.168.4.1

## Calls
### Wlan ändern:
.../change_wlan?ssid=&pass=

192.168.4.1/change_wlan?ssid=<ssid>&pass=<pwd>

## MQTT Topics ändern
.../init_mqtt_topics?
    water_level_topic=
    &pumped_topic=
    &moisture_topic=
    &pump_topic=
    &moisture_threshhold_topic=
    &pump_intervall_topic=
    &pump_duration_topic=


192.168.4.1/init_mqtt_topics?water_level_topic=Hans/Gruppe1/waterlevel&pump_topic=Hans/Gruppe1/pump&pumped_topic=Hans/Gruppe1/pumed
&moisture_topic=Hans/Gruppe1/moisture&moisture_threshhold_topic=Hans/Gruppe1/moisture_threshhold&pump_intervall_topic=Hans/Gruppe1/pump_intervall&pump_duration_topic=Hans/Gruppe1/pump_duration 