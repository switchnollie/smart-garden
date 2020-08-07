const mongoose = require("mongoose");
const User = require("./models/user");
const WateringGroup = require("./models/wateringGroup");
const Device = require("./models/device");

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
    displayName: "orchid",
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
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
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "moisture",
    logs: [
      {
        date: Date.now() - 30000,
        value: 42
      },
      {
        date: Date.now(),
        value: 42
      }
    ]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d2f515e9536fb08962ba5"),
    displayName: "Pump 1",
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "pump",
    logs: [
      {
        date: Date.now() - 30000,
        value: 1
      },
      {
        date: Date.now(),
        value: 0
      }
    ]
  },
  {
    _id: mongoose.Types.ObjectId("5f2d30f7c22d4d3103a19b22"),
    displayName: "Waterlevel Sensor 1",
    groupedBy: mongoose.Types.ObjectId("5f2d2bfe7824f2b9fd33cb66"),
    ownedBy: mongoose.Types.ObjectId("5f2d2b58d65dd0c3e0ac05e7"),
    type: "waterlevel",
    logs: [
      {
        date: Date.now() - 30000,
        value: 100
      },
      {
        date: Date.now(),
        value: 42
      }
    ]
  }
];

async function clearCollections() {
  for (let model of [User, WateringGroup, Device]) {
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

    mongoose.disconnect();
    process.exit(0);
  } catch (err) {
    console.error(err);
    mongoose.disconnect();
    process.exit(1);
  }
}

applySeeding();
