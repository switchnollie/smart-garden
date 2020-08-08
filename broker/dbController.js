const DeviceLogBucket = require("./models/deviceLogBucket");
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

module.exports = { writeLogToDb };
