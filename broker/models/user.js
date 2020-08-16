const { Schema, model } = require("mongoose");

const UserSchema = new Schema({
  hash: String,
  salt: String,
  userName: String,
  wateringGroups: [
    {
      type: Schema.Types.ObjectId,
      ref: "WateringGroup"
    }
  ]
});

module.exports = model("User", UserSchema);
