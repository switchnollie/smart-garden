const { Schema, model } = require("mongoose");

const WateringGroupSchema = new Schema({
  displayName: String,
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "User"
  },
  devices: [
    {
      type: String,
      ref: "Device"
    }
  ],
  moistureThreshold: Number,
  lastPumped: Schema.Types.Date,
  minimalPumpInterval: Number
});

module.exports = model("WateringGroup", WateringGroupSchema);
