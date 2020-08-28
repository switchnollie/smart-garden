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
  moistureThreshold: {type: Number, default: 411},
  lastPumped: Schema.Types.Date,
  minimalPumpInterval: {type: Number, default: 86400000}
});

module.exports = model("WateringGroup", WateringGroupSchema);
