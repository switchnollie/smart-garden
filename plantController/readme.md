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
	Params:
        Intervall : int