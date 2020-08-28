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
    wateringGroups: [],
    userName: "Hendrik",
    hash:
      "000eb644e0b1e196638933cb6f7121dc92cc146aa9c5f8cb9cbc0359a09988106110c088f8da5a6d91741970837754d95ad119fdc2b28699fddfcec07e4da974",
    salt: "9a24d092a5ca433c52e8979409fb637962d7cb0a611c695aa0748b7d1adfdacd"
  },
  {
    _id: mongoose.Types.ObjectId("5f39934ab02e6b565372e49d"),
    wateringGroups: [mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66")],
    userName: "Tim",
    hash:
      "49081691ecfa4e9162ab411bc887f44288f19e864ae5e271d83a0e5bfce371f41b02b60ecfb115e09da39bedead39aa030945c15bdfc1bc0cfde7cbc2fb8651b",
    salt: "a5f20df5df16afe24124b9b01fb3aa74c08e6862ea098d65f5db87f74ac2798b"
  }
];

const initialWateringGroups = [
  {
    _id: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    displayName: "Orchid",
    ownedBy: mongoose.Types.ObjectId("5f39934ab02e6b565372e49d"),
    lastPumped: nowTimestamp,
    moistureThreshold: 55,
    minimalPumpInterval: 604800000,
    devices: [
      "31cd103745c5e5c2cf15e922f3c815a87cacd75c",
      "8e09111f3b8bc9fd8829b48fbd6eaf577a921ca9",
      "389641e7f3fa334a69d2f8393da6e1ddbc31b055"
    ]
  }
];

const initialDevices = [
  {
    _id: "31cd103745c5e5c2cf15e922f3c815a87cacd75c",
    displayName: "Moisture Sensor 1",
    lastValue: 42,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f39934ab02e6b565372e49d"),
    type: "moisture",
    logBuckets: [mongoose.Types.ObjectId("5f2d6fb18fd7805f913cc40a")]
  },
  {
    _id: "8e09111f3b8bc9fd8829b48fbd6eaf577a921ca9",
    displayName: "Pump 1",
    lastValue: 1,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f39934ab02e6b565372e49d"),
    type: "pump",
    logBuckets: [mongoose.Types.ObjectId("5f2d728d1407c5e120a3980e")]
  },
  {
    _id: "389641e7f3fa334a69d2f8393da6e1ddbc31b055",
    displayName: "Waterlevel Sensor 1",
    lastValue: 42,
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f39934ab02e6b565372e49d"),
    type: "waterlevel",
    logBuckets: [mongoose.Types.ObjectId("5f2d72f56a21f09e868acf71")]
  }
];

const initialDeviceLogBuckets = [
  {
    _id: mongoose.Types.ObjectId("5f2d6fb18fd7805f913cc40a"),
    deviceId: "31cd103745c5e5c2cf15e922f3c815a87cacd75c",
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
    deviceId: "8e09111f3b8bc9fd8829b48fbd6eaf577a921ca9",
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
    deviceId: "389641e7f3fa334a69d2f8393da6e1ddbc31b055",
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
      console.log(`inserting new User, ${user.userName}`);
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
