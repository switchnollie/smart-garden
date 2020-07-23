# Server

## Config
IP: 192.168.4.1
DNS: tibs_pump_controller.de
PORT: 53

## Calls
### Wlan ändern:
.../change_wlan?ssid=""&pass=""

## MQTT Topics ändern
.../init_mqtt_topics?
    water_level_topic=""
    &pumped_topic=""
    &moisture_topic=""
    &pump_topic=""
    &moisture_threshhold_topic=""
    &pump_intervall_topic=""
    &pump_duration_topic=""