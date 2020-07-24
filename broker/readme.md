#Connection
IP-Address: 192.168.0.143
Port: 1883

#Topics:
INIT 
    -Value assigned in initialization
    -can be whatever -> needed to assign pump and plantcontroller
    -bsp. USERID/ErstePflanzengruppe(Nutzereingabe)/waterlevel

//Start webserver
MQTT_ID/webserver

//Water level of pump controller
INIT/waterlevel

//Timestamp when pumpcontroller pumped
INIT/pumped

//Moisture from plant controller
INIT/moisture

//Request pump to pumpcontroller
INIT/PUMP

//Threshold when pumpcontroller shall pump
INIT/MOISTURE_THRESHHOLD

//Intervall when pumpcontroller shall pump
INIT/PUMP_INTERVALL

//Time duration for pumping
INIT/PUMP_DURATION

