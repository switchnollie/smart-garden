const { Schema, model } = require("mongoose");

const WateringGroupSchema = new Schema({
  displayName: String,
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "User"
  },
  devices: [
    {
      type: Schema.Types.ObjectId,
      ref: "Device"
    }
  ]
});

module.exports = model("WateringGroup", WateringGroupSchema);
