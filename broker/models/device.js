const { Schema, model } = require("mongoose");

const DeviceSchema = new Schema({
  _id: String,
  type: String,
  displayName: String,
  lastValue: Schema.Types.Mixed,
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
