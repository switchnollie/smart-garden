# Ideensammlung TIBS Projekt

## Smart Garden

#### Hardware

##### 1. Pflanzenstation

- Feuchtigkeitssensor für die Pflanze
- AALEC
- Breadboard
- Jumper Kabel

##### 2. Pumpencontroller

- AALEC
- Pumpe
- Breadboard
- Jumper Kabel
- PVC Schläuche
- Relais zum Ansteuern der Pumpe



#### Web Interface

- Dashboard Page: 
  - Aktuelle Feuchtigkeitswerte der Pflanze(n)
  - Historie der Wasserabgaben
- Control Page: 
  - Buttons zum manuellen Ansteuern der Pumpe
  - Automatisierungsoptionen:
    - Option festes Intervall
    - Option nach Feuchtigkeitsstand
- Deployment zusammen mit MQTT-Broker
- Deployment zusammen mit Datenbank z.B. MongoDB



### TODO

- Prüfen was schon an Hardware rumliegt
- Hardware bestellen und kaufen
- Systemarchitektur beschreiben
  - MQTT Topics definieren
  - Sequenzdiagramme aufschreiben (einzelne Komponenten, Nachrichtenaustausch, Persistenz)
- Platform IO zum laufen bringen mit Example Sketches
- gemeinsames Repository aufsetzen:
  - plantController
  - 
    - Platform IO
  - pumpController
    - Platform IO
  - server
    - Mosca MQTT-Broker
    - Express Webserver
    - Datenbank