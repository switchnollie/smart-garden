const DeviceLogBucket = require("./models/device");
const getDayDate = require("./helpers/getDayDate");

async function writeLogToDb(deviceId, pckg) {
  const newLog = {
    val: pckg.payload.toString(),
    time: Date.now()
  };

  DeviceLogBucket.updateOne(
    {
      deviceId,
      nsamples: {
        $lt: 200,
        day: getDayDate()
      }
    },
    {
      $push: { samples: newLog },
      $min: { first: newLog.time },
      $max: { last: newLog.time },
      $inc: { nsamples: 1 }
    },
    { upsert: true }
  );
}

module.exports = { writeLogToDb };
