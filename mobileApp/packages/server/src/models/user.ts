import { Schema, model, Document } from "mongoose";
import { IWateringGroupModel } from "./wateringGroup";

export interface IUserModel extends Document {
  hash: string;
  salt: string;
  userName: string;
  wateringGroups: IWateringGroupModel[];
}

const UserSchema = new Schema({
  hash: String,
  salt: String,
  userName: String,
  wateringGroups: [
    {
      type: Schema.Types.ObjectId,
      ref: "WateringGroup",
    },
  ],
});

export default model<IUserModel>("User", UserSchema);
