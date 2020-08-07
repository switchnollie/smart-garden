const { Schema, model } = require("mongoose");

const DeviceSchema = Schema({
  type: String,
  displayName: String,
  logBuckets: [
    {
      type: Schema.Types.ObjectId,
      ref: "DeviceLogBucket"
    }
  ],
  groupedBy: {
    type: Schema.Types.ObjectId,
    ref: "WateringGroup"
  },
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "User"
  }
});

module.exports = model("Device", DeviceSchema);
