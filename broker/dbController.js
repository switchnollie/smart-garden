const DeviceLogBucket = require("./models/deviceLogBucket");
const WateringGroup = require("./models/wateringGroup");
const Device = require("./models/device");
const getDayString = require("./helpers/getDayString");

async function writeLogToDb(deviceId, pckg) {
  const newLog = {
    val: pckg.payload.toString(),
    time: Date.now()
  };

  const day = getDayString();
  try {
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
  } catch (error) {
    console.error(`Error updating DeviceLogs for device ${deviceId}`, error);
  }
  try {
    await Device.updateOne(
      { _id: deviceId },
      {
        lastValue: newLog.val
      }
    );
  } catch (error) {
    console.error(`Error updating lastValue for device ${deviceId}`, error);
  }
}

/**
 * Check if the measured moisture value has fallen below the threshold
 */
async function checkMoistureThreshold(groupId, moistureVal) {
  try {
    const group = await WateringGroup.findOne(
      { _id: groupId },
      "moistureThreshold"
    ).exec();
    return group.moistureThreshold > parseInt(moistureVal, 10);
  } catch (error) {
    console.error(error);
    return null;
  }
}

async function getLastPumped(groupId) {
  try {
    const group = await WateringGroup.findOne(
      { _id: groupId },
      "lastPumped"
    ).exec();
    return group.lastPumped;
  } catch (error) {
    console.error(`Error getting lastPumped of group ${groupId}`, error);
    return null;
  }
}

async function updateLastPumped(groupId) {
  console.log(`updating last pumped of group ${groupId} to ${Date.now()}`);
  try {
    await WateringGroup.updateOne(
      { _id: groupId },
      { lastPumped: new Date() }
    ).exec();
    console.log(`updated last pumped of group ${groupId}`);
  } catch (error) {
    console.error(`Error updating lastPumped for group ${groupId}`, error);
  }
}

async function getPumpsForGroup(groupId) {
  try {
    return (
      await Device.find({ type: "pump", groupedBy: groupId }, "_id").exec()
    ).map(doc => doc._id);
  } catch (error) {
    console.error(`Error getting pumps for group ${groupId}`, error);
    return null;
  }
}

module.exports = {
  writeLogToDb,
  checkMoistureThreshold,
  getLastPumped,
  updateLastPumped,
  getPumpsForGroup
};
