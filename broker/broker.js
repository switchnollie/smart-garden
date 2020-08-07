const { createServer } = require("net");
const Aedes = require("aedes");
const mongoose = require("mongoose");
const logging = require("aedes-logging");
const mongoPersistence = require("aedes-persistence-mongodb");
const mqemitter = require("mqemitter-mongodb");

const MQTT_PORT = process.env.MQTT_PORT || 1883;

function initBroker(mongoConnection) {
  const db = mongoConnection.useDb("logPersistence").db;
  const aedes = Aedes({
    persistence: mongoPersistence({ db }),
    mq: mqemitter({ db })
  });

  const mqttServer = createServer(aedes.handle);

  logging({ instance: aedes, servers: [mqttServer] });

  mqttServer.listen(MQTT_PORT, "0.0.0.0", () => {
    console.log("broker started and listening on port", MQTT_PORT);
  });

  aedes.authorizePublish = (client, packet, callback) => {
    const topicParts = packet.topic.split("/");
    const clientId = client.id;
    if (topicParts.length < 4) {
      const error = new Error("topics must have 4 levels: <userid>/<groupid>/<deviceid>/+");
      console.error(error);
      return callback(error);
    }
    const topicDeviceId = topicParts[2];

    if (topicDeviceId !== clientId) {
      const error = new Error("the mqtt client id doesn't match the deviceId in the message topic");
      console.error(error);
      return callback(error);
    }
    console.log("authorized!")
    // All validation checks passed, return null to authorize
    return callback(null);
  }

  // react to moisture messages
  aedes.mq.on("+/+/+/moisture", (package, cb) => {
    const stringMessage = package.payload.toString();
    console.log(stringMessage);
  });
}

module.exports = { initBroker };
