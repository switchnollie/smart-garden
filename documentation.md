# Smart Garden

> Ein Projekt von Tim Weise und Hendrik Höing im Fach Technische Internetbasierte Systeme im Sommersemester 2020

## Zielsetzung

Ziel des Projekts ist, ein autonomes Bewässerungssystem umzusetzen. 
Dies soll durch eine Überwachung des Feuchtigkeitsgehalts der Pflanzenerde mithilfe von Feuchtigkeitssensoren und einer peristaltischen Pumpe, welche über Schläuche Wasser von einem Tank in die Pflanzenerde befördert, realisiert werden.

Zusätzlich soll der Wasserstand im Tank über einen Wasserstandsensor gemessen werden und übliche Parameter und Metriken über eine mobile Webanwendung einsehbar und steuerbar sein.

## Systemarchitektur

Auf der Hardwareseite besteht das System aus Pumpencontrollern mit jeweils einer Pumpe und einem Wasserstandsensor und aus Pflanzencontrollern mit jeweils einem Feuchtigkeitssensor.

Diese kommunizieren mittels MQTT über TLS verschlüsselt mit einem Broker, welcher Regeln für das Auslösen weiterer Aktionen implementiert.

Darüberhinaus lassen sich Systemkonfigurationen über eine mobile Webanwendung vornehmen, über welche auch Echtzeitwerte des Systems einsehbar sind. 

Konkret sind die über die Webanwendung konfigurierbaren Parameter die Feuchtigkeitsschwelle, ab der spätestens bewässert wird und das minimale Pumpintervall, d.h. wann spätestens nach einer Bewässerung erneut bewässert wird, auch wenn die Feuchtigkeitssensoren keine Messwerte unter der konfigurierten Schwelle melden.

Sowohl die Messdaten, als auch die Konfigurationen der Systemnutzer und deren Bewässerungsgruppen und Geräte (Pflanzencontroller, Pumpencontroller und Wasserstandssensor werden als Einzelgeräte konfiguriert) werden in einer zentralen Datenbank erfasst. 
Es handelt sich dabei um eine extern gehostete MongoDB-Instanz, die nach dem Prinzip _Database as a Service_ bereitgestellt und genutzt wird.

Für die Konfiguration und den Abruf aller Nutzer- und Gerätedaten steht des Weiteren ein Anwendungsserver mit REST-API zur Verfügung, welcher darüberhinaus jedoch auch als Webserver für die mobile Webanwendung fungiert. Wie auch der Broker befindet sich dieser Server hinter einem Reverse-Proxy/ Application Gateway, welcher Verbindungen nur verschlüsselt über HTTPS akzeptiert.

Dieses System aus MQTT-Broker, Anwendungsserver und Reverse-Proxy ist mithilfe von Containern realisiert, die über Docker Compose als Microservices miteinander kommunizieren. 

Das System ist auf einem Digital Ocean Droplet (einem sog. _Virtual Private Server_) öffentlich deployt und unter `smartgarden.timweise.com` erreichbar. Es lässt sich aber auch über die Kubernetes Konfigurationen in einem Kubernetes Cluster deployen, um Hochverfügbarkeit zu garantieren.

## Ablauf

**TODO**: Sequenzdiagramme: Lebenslinien sind (Pflanzencontroller, Pumpencontroller, HAProxy, Anwendungsserver, Web Client, Mongo-Datenbank)

- **TODO**: Sequenzdiagramm Parametrisierung/ Ersteinrichtung
- **TODO**: Video/GIF Parametrisierung/ Ersteinrichtung

- **TODO**: Sequenzdiagramm Feuchtigkeitsschwelle überschritten
- **TODO**: Video/GIF Feuchtigkeitsschwelle überschritten

- **TODO**: Sequenzdiagramm Firmwareupdate
- **TODO**: Video/GIF Firmwareupdate

## Komponenten

### Pflanzencontroller

- **TODO**: _Hendrik_

### Pumpencontroller

- **TODO**: _Hendrik_

### Serverpart (Microservices)

- **TODO**: _Tim_

### Web-Frontend

- **TODO**: _Tim_




