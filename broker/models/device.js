const { Schema, model } = require("mongoose");

const DeviceSchema = Schema({
  type: String, // TODO: This is more an enum type
  logs: [{
    date: Date,
    value: Number,
  }],
});

module.exports = model("Device", DeviceSchema);