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

Die Authorisierung erfolgt durch das Matching mit einem definierten Topicformat, welches die Form `<userId>/<groupId>/<deviceId>/<actionType/telemetryType>` hat. 
Die über `groupId` identifizierten Gruppen sind dabei Zusammenstellungen mehrerer Controller, welche ein separates Bewässerungssystem bilden (ein Bewässerungssystem kann aus mehreren Pflanzen- und Pumpencontrollern bestehen).
Die implementierten `telemetryTypes` sind `pump` (Pumpe aktivieren), `moisture` (neuer Feuchtigkeitsmesswert) und `waterLevel` (neuer Wasserstands-Messwert).

Sowohl die Messdaten, als auch die Konfigurationen der Systemnutzer und deren Bewässerungsgruppen und Geräte (Pflanzencontroller, Pumpencontroller und Wasserstandssensor werden als Einzelgeräte konfiguriert) werden in einer zentralen Datenbank erfasst. 
Es handelt sich dabei um eine extern gehostete MongoDB-Instanz, die nach dem Prinzip _Database as a Service_ bereitgestellt und genutzt wird.

Für die Konfiguration und den Abruf aller Nutzer- und Gerätedaten steht des Weiteren ein Anwendungsserver mit REST-API zur Verfügung, welcher darüberhinaus jedoch auch als Webserver für die mobile Webanwendung fungiert. Wie auch der Broker befindet sich dieser Server hinter einem Reverse-Proxy/ Application Gateway, welcher Verbindungen nur verschlüsselt über HTTPS akzeptiert.

Dieses System aus MQTT-Broker, Anwendungsserver und Reverse-Proxy ist mithilfe von Containern realisiert, die über Docker Compose als Microservices miteinander kommunizieren. 

Das System ist auf einem Digital Ocean Droplet (einem sog. _Virtual Private Server_) öffentlich deployt und unter `smartgarden.timweise.com` erreichbar. Es lässt sich aber auch über die Kubernetes Konfigurationen in einem Kubernetes Cluster deployen, um Hochverfügbarkeit zu garantieren.

## Ablauf

**TODO**: Sequenzdiagramme: Lebenslinien sind (Pflanzencontroller, Pumpencontroller, HAProxy, Anwendungsserver, Web Client, MQTT-Broker, Mongo-Datenbank)

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

### Web-Anwendung

Das Webfrontend ist mit den Frameworks React.js und Ionic realisiert.
Wie auch der Anwendungsserver ist das Frontend in Typescript realisiert, um über statische Typisierung möglichst viele potentielle Fehler zur Compilezeit abzufangen.
Als Single Page Webanwendung lässt sich das kompilierte App-Bundle in ein Verzeichnis des Webservers kopieren und über einen Handler für statische Dateien ausliefern.

```ts
/* 
Für alle Routen, die nicht mit /api beginnen liefert der 
Anwendungsserver die index.html aus, welche den Einstiegspunkt in die Webanwendung darstellt.
*/
app.use(express.static(path.join(__dirname, "..", "public")));
app.get("*", (req, res) => {
  res.sendFile(path.join(__dirname, "..", "public", "index.html"));
});
```

Über ein Formular in der Webanwendung authentifiziert sich der Nutzer außerdem über eine Login-Route gegenüber dem Server, der im Falle korrekt übermittelter Nutzer-Credentials einen JSON-Webtoken ausstellt.

Die clientseitigen Routen sind geschützt und können nur aufgerufen werden, falls ein valides Token im Browserspeicher (localStorage) zur Verfügung steht, die API-Routen sind darüberhinaus durch die Passport-Middleware geschützt, welche das Token auf Validität überprüft.

Für das Auslösen einer Bewässerung wird im Anwendungsserver ein Endpunkt unter `/api/action/pump` bereitgestellt. 
Bei eingehenden, authentifizierten Anfragen löst der Callback ein MQTT-Publish auf dem Topic aller der Gruppe zugeordneten Pumpen aus.




