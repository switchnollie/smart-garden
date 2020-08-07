const { Schema, model } = require("mongoose");

const UserSchema = new Schema({
  displayName: String,
  wateringGroups: [
    {
      type: Schema.Types.ObjectId,
      ref: "WateringGroup"
    }
  ]
});

module.exports = model("User", UserSchema);
