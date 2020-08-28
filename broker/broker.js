const { createServer } = require("net");
const Aedes = require("aedes");
const logging = require("aedes-logging");
const mongoPersistence = require("aedes-persistence-mongodb");
const mqemitter = require("mqemitter-mongodb");
const {
  writeLogToDb,
  checkMoistureThreshold,
  getLastPumped,
  updateLastPumped,
  getPumpsForGroup
} = require("./dbController");
const { parseTopic } = require("./helpers/topicSchema");

const MQTT_PORT = process.env.MQTT_PORT || 1883;

function initBroker(db) {
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
    const { deviceId, length } = parseTopic(packet.topic);
    const clientId = client.id;
    if (length < 4) {
      const error = new Error(
        "topics must have 4 levels: <userid>/<groupid>/<deviceid>/+"
      );
      console.error(error);
      return callback(error);
    }
    const clientIdWaterlevel = clientId + "-w";
    if (
      deviceId !== clientId &&
      deviceId !== clientIdWaterlevel &&
      !clientId.startsWith("admin-")
    ) {
      const error = new Error(
        "the mqtt client id doesn't match the deviceId in the message topic"
      );
      console.error(error);
      return callback(error);
    }
    // All validation checks passed, return null to authorize
    return callback(null);
  };

  aedes.mq.on("+/+/+/waterlevel", async (pkg, cb) => {
    const { deviceId } = parseTopic(pkg.topic);
    writeLogToDb(deviceId, pkg);
    cb();
  });

  aedes.mq.on("+/+/+/pump", async (pkg, cb) => {
    const { deviceId, groupId } = parseTopic(pkg.topic);
    writeLogToDb(deviceId, pkg);
    updateLastPumped(groupId);
    cb();
  });

  // react to moisture messages
  aedes.mq.on("+/+/+/moisture", async (pkg, cb) => {
    const { userId, groupId, deviceId } = parseTopic(pkg.topic);
    writeLogToDb(deviceId, pkg);
    const lastPumped = +(await getLastPumped(groupId));
    const isMoistureThresholdReached = await checkMoistureThreshold(
      groupId,
      pkg.payload.toString()
    );
    // Only pump if the threshold is exceeded and the last pump is more than 2 minutes ago.
    if (isMoistureThresholdReached && Date.now() - lastPumped > 5000) {
      console.log(
        `moisture threshold reached, last pump ${
          Date.now() - lastPumped
        } ms ago`
      );
      // Find corresponding pumps
      const pumpIds = await getPumpsForGroup(groupId);
      pumpIds.forEach(pumpId => {
        aedes.mq.emit({
          topic: `${userId}/${groupId}/${pumpId}/pump`,
          payload: 1
        });
      });
    }
    cb();
  });
}

module.exports = { initBroker };
