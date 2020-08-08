import { Schema, model, Document } from "mongoose";
import { IWateringGroupModel } from "./wateringGroup";

export interface IUserModel extends Document {
  displayName: string;
  wateringGroups: IWateringGroupModel[];
}

const UserSchema = new Schema({
  displayName: String,
  wateringGroups: [
    {
      type: Schema.Types.ObjectId,
      ref: "WateringGroup",
    },
  ],
});

export default model<IUserModel>("User", UserSchema);
