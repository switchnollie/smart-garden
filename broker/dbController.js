const DeviceLogBucket = require("./models/deviceLogBucket");
const WateringGroup = require("./models/wateringGroup");
const getDayString = require("./helpers/getDayString");

async function writeLogToDb(deviceId, pckg) {
  const newLog = {
    val: pckg.payload.toString(),
    time: Date.now()
  };

  const day = getDayString();

  await DeviceLogBucket.updateOne(
    {
      deviceId,
      nsamples: {
        $lt: 200
      },
      day
    },
    {
      $push: { samples: newLog },
      $min: { first: newLog.time },
      $max: { last: newLog.time },
      $inc: { nsamples: 1 },
      deviceId,
      day
    },
    { upsert: true }
  );
}

/**
 * Check if the measured moisture value has fallen below the threshold
 */
async function checkMoistureThreshold(groupId, moistureVal) {
  const group = await WateringGroup.findOne(
    { _id: groupId },
    "moistureThreshold"
  ).exec();
  return group.moistureThreshold > parseInt(moistureVal, 10);
}

async function getLastPumped(groupId) {
  const group = await WateringGroup.findOne(
    { _id: groupId },
    "lastPumped"
  ).exec();
  return group.lastPumped;
}

async function updateLastPumped(groupId) {
  console.log(`updating last pumped of group ${groupId} to ${Date.now()}`);
  await WateringGroup.updateOne(
    { _id: groupId },
    { lastPumped: new Date() }
  ).exec();
}

module.exports = {
  writeLogToDb,
  checkMoistureThreshold,
  getLastPumped,
  updateLastPumped
};
