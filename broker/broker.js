const Aedes = require("aedes");
const mongoose = require("mongoose");
const logging = require("aedes-logging");
const mongoPersistence = require("aedes-persistence-mongodb");
const mqemitter = require("mqemitter-mongodb");

const MQTT_PORT = process.env.MQTT_PORT || 1883;

function initBroker(db) {
  
  // const aedes = Aedes({
  //   persistence: mongoPersistence({ db }),
  //   mq: mqemitter({ db })
  // });

  const aedes = Aedes();

  const mqttServer = require("net").createServer(aedes.handle);

  logging({ instance: aedes, servers: [mqttServer] });

  mqttServer.listen(MQTT_PORT, "0.0.0.0", () => {
    console.log("broker started and listening on port", MQTT_PORT);
  });
}

module.exports = { initBroker };
