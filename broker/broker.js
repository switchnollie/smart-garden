const Aedes = require("aedes");
const mongoose = require('mongoose');
const logging = require("aedes-logging");
const mongoPersistence = require("aedes-persistence-mongodb");
const mqemitter = require('mqemitter-mongodb');

const MONGO_URI = process.env.MONGO_URI;
const MQTT_PORT = process.env.MQTT_PORT || 1883;
const MONGO_DATABASE = process.env.MONGO_DATABASE;

function initBroker() {
  const aedes = Aedes({
    persistence: mongoPersistence({
        db: mongoose.connection.useDb(MONGO_DATABASE).db,
    }),
    mq: mqemitter({
        url: MONGO_URI
    }),
  });

  const mqttServer = require("net").createServer(aedes.handle);

  logging({ instance: aedes, servers: [mqttServer] });

  mqttServer.listen(MQTT_PORT, "0.0.0.0", () => {
    console.log("broker started and listening on port", MQTT_PORT);
  });
}


module.exports = { initBroker };