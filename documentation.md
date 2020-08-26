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
Der implementierte `actionType` ist `pump` (Pumpe aktivieren) und die implementierten `telemetryTypes` sind `moisture` (neuer Feuchtigkeitsmesswert) und `waterLevel` (neuer Wasserstands-Messwert).

Sowohl die Messdaten, als auch die Konfigurationen der Systemnutzer und deren Bewässerungsgruppen und Geräte (Pflanzencontroller, Pumpencontroller und Wasserstandssensor werden als Einzelgeräte konfiguriert) werden in einer zentralen Datenbank erfasst. 
Es handelt sich dabei um eine extern gehostete MongoDB-Instanz, die nach dem Prinzip _Database as a Service_ bereitgestellt und genutzt wird.

Für die Konfiguration und den Abruf aller Nutzer- und Gerätedaten steht des Weiteren ein Anwendungsserver mit REST-API zur Verfügung, welcher darüberhinaus jedoch auch als Webserver für die mobile Webanwendung fungiert. Wie auch der Broker befindet sich dieser Server hinter einem Reverse-Proxy/ Application Gateway, welcher Verbindungen nur verschlüsselt über HTTPS akzeptiert.

Dieses System aus MQTT-Broker, Anwendungsserver und Reverse-Proxy ist mithilfe von Containern realisiert, die über Docker Compose als Microservices miteinander kommunizieren. 

Das System ist auf einem Digital Ocean Droplet (einem sog. _Virtual Private Server_) öffentlich deployt und unter `smartgarden.timweise.com` erreichbar. Es lässt sich aber auch über die Kubernetes Konfigurationen in einem Kubernetes Cluster deployen, um Hochverfügbarkeit zu garantieren.

Eine Übersicht über die Systemarchitektur bietet folgendes Komponentendiagramm:

<img src="./documentationAssets/Architecture.png" alt="Systemarchitektur Smart Garden" style="max-height:75%;" />

## Ablauf

#### Initialisierung
In der unteren Abbildung ist ein Sequenzdiagramm der Initialisierung zu sehen.

Zur Einrichtung der Controller ist ein lokaler Webserver mit DNS implementiert. Die Controller laufen im Access-Point und Station Modus, wodurch der ESP in der Lage ist einen Access-Point anzubieten und gleichzeitig Daten über WLAN an den Server zu senden. 

<img src="./documentationAssets/initialization.png" alt="Sequenzdiagramm Funktionsweise System" style="max-height:75%;" />

Über die Internetadresse `esp8266.local/wlan` kann der Kunde nach dem Einwählen in das ESP-WiFi die Initialisierungsroutine starten.

Die relevanten Webseiten werden mit dem SPIFFS-Speicher ausgeliefert, einem internen Speicher auf den Microcontrollern.

Im ersten Schritt wird das Passwort des Access-Points verändert, da dieses bei der Erstauslieferung immer gleich ist und somit Sicherheitsrisiken aufwirft.
Zusätzlich werden die WLAN-Verbindungsdaten eingetragen, sodass der ESP in der Lage ist, relevante Daten (bspw. Nutzerdaten) an den Server zu senden.
Die entsprechenden Daten werden intern auf dem EEPROM-Speicher des ESP gespeichert, sodass bei einem Neustart des Geräts alle relevanten Daten zur Verfügung stehen.

Bei erfolgreichem Verbinden mit dem WLAN wird der Nutzer auf eine Seite weitergeleitet, in der er sich einloggen soll. Die Nutzerdaten werden an den Server mit einem POST HTTPS Request gesendet und die erhaltene Nutzer ID vom Server intern gespeichert. 

Im nächsten Schritt kann der Nutzer eine Pflanzengruppe angeben. Um das System skalierbar zu halten, wird diese Möglichkeit geboten, damit der Nutzer verschiedene Pflanzengruppen unabhängig voneinander nutzen kann. Dies kann beispielsweise für verschiedene Pflanzen oder verschiedene Gärten genutzt werden.

Durch die Nutzer ID, die Pflanzengruppe und die `ESP Flash Chip ID` wird das MQTT-Topic gebaut und intern im EEPORM gespeichert.

- **TODO**: Video/GIF Parametrisierung/ Ersteinrichtung
  
### Funktionsweise

In der unteren Abbildung ist ein Sequenzdiagramm zu sehen, welches die Funktionsweise und Kommunikationsablauf des Systems beschreibt.
<img src="./documentationAssets/communication_v2.png" alt="Sequenzdiagramm Funktionsweise System" style="max-height:75%;" />

Als erstes wird bei beiden Controllern der Webserver gestartet und sich danach mit dem WLAN verbunden. 

Nach diesen Schritten versuchen diese sich als `MQTT Client` bei dem `MQTT-Broker` anzumelden. Jede Kommunikation an die API durchläuft einen HAProxy. 
Dieser dient dazu eine verschlüsselte Kommunikation sicherzustellen und je nach Port (Port 8883 für MQTT und Port 443 für HTTPS) den entsprechenden Request weiterzuleiten. 

Nach dem erfolgreichen Verbinden mit dem MQTT-Broker subscribed der Pumpencontroller am `PUMP_TOPIC`, welches genutzt wird um die peristaltischen Pumpe anzusteuern. 

Nun wird die `Ticker-Libary` von beiden Controllern benutzt, um in einem gewissen Zeitintervall jeweils den Feuchtigkeitswert der Pflanze und den Wasserstand des Wasserbehälters zu publishen.

Auf diesen beiden `MQTT-Topics` subscribed der Anwendungsserver. Vorab liest er jedoch die Daten aus der `Mongo-Datenbank`, sodass er dem `Web-Client` die entsprechenden Daten anzeigen.

Der `Web-Client` kann über den Proxy die Web-Applikation anfragen, sich einloggen, seine Daten einsehen und folgende Werte festlegen: einen Schwellwert, ab welcher Feuchtigkeit die Pumpe die Pflanze bewässern soll und ein Intervall, in dem gepumpt wird, sollte dies davor nicht passiert sein. Diese Werte werden pro Benutzer in der `Mongo-Datenbank` abgespeichert. 

Zusätzlich besteht die Möglichkeit die Pumpe direkt anzusteuern. Jeder Pump-Befehl ist eine Nachricht an den `MQTT-Broker` auf dem `PUMP_TOPIC`.

### Update
Um den Kunden neue Versionen bieten zu können, läuft ein `Update-Server` auf den Microcontrollern. Nach Downloaden der Binärdatei kann diese über `esp8266.local/update` hochgeladen und die neue Version aufgespielt werden.
**TODO** GIF vom Update

### WiFi Management
Zur Kapselung und Verbesserung der Code Qualität ist eine Headerdatei implementiert, die das komplette WiFi Management übernimmt.
Diese beinhaltet den Webserver, DNS, Verbindungsmanagement mit dem WLAN und das Senden von Daten per `HTTPS Client` an den Server.
Über den Webserver kann die Initialisierung erfolgen. Zusätzlich wird dieser dauerhaft betrieben, um Änderungen am WLAN oder an den `MQTT-Topics` und das Firmware-Update zu erlauben.

### MQTT Management
Zur Verbindung und Kommunikation über MQTT wird der MQTT-Client `PubSubClient` verwendet. Um eine sichere Verbindung zu gewährleisten wird `WifiClientSecure` benutzt, ein Client, der eine Authorisierung anbietet. Zur Authorisierung wird ein Zertifikat, welches im SPIFFS speicher hinterlegt ist und ein zugehöriger `Fingerprint` genutzt. Nach erfolgreicher Authorisierung und Verbindung mit dem Server, wird eine Nutzer ID für den `PubSubClient` generiert. Hierfür wird die `ESP Flash Chip ID` genutzt. Mit der Nutzer ID wird eine Verbindung zum `MQTT-Broker` (TODO referenz?) aufgebaut und die entsprechenden `MQTT-Topics` abonniert.


- **TODO**: Video/GIF Manuelles Ansteuern der Pumpe über die App

## Komponenten

### Pflanzen-/Pumpencontroller

**TODO** Schaltplan, technische beschreibung
Der Pflanzencontroller veröffentlicht mit einem Feuchtigkeitssensor Daten über `MQTT`. Der Pumpencontroller veröffentlich einerseits den Wasserstand des Wasserbehälters über `MQTT` und besitzt zusätzlich eine peristaltischen Pumpe, die über ein `MQTT-Topic` ausgelöst werden kann, welches der Pumpencontroller abonniert hat.

### MQTT-Broker

Der MQTT-Broker ist mit Node.js implementiert und baut auf der Aedes-Bibliothek auf.
Aedes ist der Nachfolger von Mosca, einer sehr beliebten Open Source MQTT-Broker-Implementierung in Node.js.

Ein Vorteil von Aedes ist, dass man mithilfe einer Plugin-Bibliothek eine Persistenzschicht für die Message Queue übergeben kann. 
In unserem Fall haben wir hierfür dieselbe Mongo-Datenbank, welche auch für die Nutzer- und Gerätedaten verwendet wird eingebunden, dies würde nach dem Microservice-Pattern in einer späteren Produktiteration jedoch durch eine dedizierte, weitere Datenbank nur für die Message Queue ersetzt werden.

Über die Handler der Message Queue sind die Regeln implementiert, wann auf `<userid>/<groupid>/<deviceid>/pump` gepublisht wird und wie die Logs in die MongoDB geschrieben werden.

Für die Datenbankinteraktion kommt der Object Document Mapper (ODM) Mongoose zum Einsatz.

```js
/* 
Für alle Nachrichten auf den Topics +/+/+/pump (die + stehen für Wildcards 
("Alle-Platzhalter") auf der jew. Ebene) wird der Callback ausgeführt.
Die Logs werden nach dem vom ODM vorgegebenen Schema 
*/
aedes.mq.on("+/+/+/pump", async (pkg, cb) => {
  const { deviceId, groupId } = parseTopic(pkg.topic);
  writeLogToDb(deviceId, pkg);
  updateLastPumped(groupId);
  cb();
});
```

### Web-Anwendung

Das Webfrontend wurde mit den Frameworks React.js und Ionic realisiert.
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


