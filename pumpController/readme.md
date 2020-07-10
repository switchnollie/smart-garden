# Server

## Config
IP: 192.168.4.1
DNS: tibs_pump_controller.de
PORT: 53

## Calls
### Wlan ändern:
Pfad: .../change_wlan
Params: 
    ssid : String
    pass : String
### Pumpen:
	Pfad: .../pump
### Pumpintervall setzen:
    Pfad: .../pump_intervall
    Params:
        intervall : int (d)
### Pumpdauer setzen:
    Pfad: .../pump_duration
    Params:
        durations : int (s)
### Humidity threshhold setzen
    Pfad: .../humidity_threshhold
    Params: 
        threshhold : int