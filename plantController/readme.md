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


### MQTT Topics ändern:
.../init_mqtt_topics?
    username=
    group=

192.168.4.1/init_mqtt_topics?username=Hannes&group=Gruppe1

## AP Passwort ändern:
../change_ap_password?
    pass=