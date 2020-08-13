const mongoose = require("mongoose");
const getDayString = require("./helpers/getDayString");
const User = require("./models/user");
const WateringGroup = require("./models/wateringGroup");
const Device = require("./models/device");
const DeviceLogBucket = require("./models/deviceLogBucket");

const nowTimestamp = Date.now();
const lastTimestamp = nowTimestamp - 5000;

const initialUsers = [
  {
    _id: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    displayName: "Tim",
    wateringGroups: [mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66")]
  }
];

const initialWateringGroups = [
  {
    _id: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    displayName: "Orchid",
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    lastPumped: nowTimestamp,
    moistureThreshold: 55,
    minimalPumpInterval: 604800000,
    devices: [
      mongoose.Types.ObjectId("5f2d2f46c254098c1222a484"),
      mongoose.Types.ObjectId("5f2d2f515e9536fb08962ba5"),
      mongoose.Types.ObjectId("5f2d30f7c22d4d3103a19b22")
    ]
  }
];

const initialDevices = [
  {
    _id: mongoose.Types.ObjectId("5f2d2f46c254098c1222a484"),
    displayName: "Moisture Sensor 1",
    lastValue: 42,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "moisture",
    logBuckets: [mongoose.Types.ObjectId("5f2d6fb18fd7805f913cc40a")]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d2f515e9536fb08962ba5"),
    displayName: "Pump 1",
    lastValue: 1,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "pump",
    logBuckets: [mongoose.Types.ObjectId("5f2d728d1407c5e120a3980e")]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d30f7c22d4d3103a19b22"),
    displayName: "Waterlevel Sensor 1",
    lastValue: 42,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "waterlevel",
    logBuckets: [mongoose.Types.ObjectId("5f2d72f56a21f09e868acf71")]
  }
];

const initialDeviceLogBuckets = [
  {
    _id: mongoose.Types.ObjectId("5f2d6fb18fd7805f913cc40a"),
    deviceId: mongoose.Types.ObjectId("5f2d2f46c254098c1222a484"),
    nsamples: 2,
    day: getDayString(),
    first: nowTimestamp,
    last: lastTimestamp,
    samples: [
      { val: 42, time: lastTimestamp },
      { val: 42, time: nowTimestamp }
    ]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d728d1407c5e120a3980e"),
    deviceId: mongoose.Types.ObjectId("5f2d2f515e9536fb08962ba5"),
    nsamples: 2,
    day: getDayString(),
    first: nowTimestamp,
    last: lastTimestamp,
    samples: [
      { val: 1, time: lastTimestamp },
      { val: 1, time: nowTimestamp }
    ]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d72f56a21f09e868acf71"),
    deviceId: mongoose.Types.ObjectId("5f2d30f7c22d4d3103a19b22"),
    nsamples: 2,
    day: getDayString(),
    first: nowTimestamp,
    last: lastTimestamp,
    samples: [
      { val: 42, time: lastTimestamp },
      { val: 42, time: nowTimestamp }
    ]
  }
];

async function clearCollections() {
  for (const model of [User, WateringGroup, Device, DeviceLogBucket]) {
    try {
      await model.collection.drop();
    } catch (e) {
      if (e.code === 26) {
        console.log("namespace %s not found", model.collection.name);
        console.log(
          "This usually means that the collection has already been dropped"
        );
      } else {
        throw e;
      }
    }
  }
}

async function applySeeding() {
  console.log(
    `Connecting to database ${process.env.MONGO_DATABASE} on ${process.env.MONGO_HOST}...`
  );
  try {
    await mongoose.connect(
      `${process.env.MONGO_URI}/${process.env.MONGO_DATABASE}`,
      {
        useNewUrlParser: true,
        useUnifiedTopology: true
      }
    );

    console.log("Clearing all collections...");
    await clearCollections();

    const insertUsersBatch = initialUsers.map(user => {
      console.log(`inserting new User, ${user.displayName}`);
      return new User(user).save();
    });
    await Promise.all(insertUsersBatch);

    const insertWateringGroupsBatch = initialWateringGroups.map(group => {
      console.log(`inserting new WateringGroup, ${group.displayName}`);
      return new WateringGroup(group).save();
    });
    await Promise.all(insertWateringGroupsBatch);

    const insertDevicesBatch = initialDevices.map(device => {
      console.log(`inserting new Device, ${device.displayName}`);
      return new Device(device).save();
    });
    await Promise.all(insertDevicesBatch);

    const insertDeviceLogBucketsBatch = initialDeviceLogBuckets.map(bucket => {
      console.log(`inserting new Device Log Bucket, ${bucket._id}`);
      return new DeviceLogBucket(bucket).save();
    });
    await Promise.all(insertDeviceLogBucketsBatch);

    mongoose.disconnect();
    process.exit(0);
  } catch (err) {
    console.error(err);
    mongoose.disconnect();
    process.exit(1);
  }
}

applySeeding();
