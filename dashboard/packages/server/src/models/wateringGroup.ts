import { Schema, model, Document } from "mongoose";
import { IDeviceModel } from "./device";

export interface IWateringGroupModel extends Document {
  displayName: string;
  ownedBy: string;
  devices: IDeviceModel["_id"];
  moistureThreshold: number;
  lastPumped: Date;
}

const WateringGroupSchema = new Schema({
  displayName: String,
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "User",
  },
  devices: [
    {
      type: Schema.Types.ObjectId,
      ref: "Device",
    },
  ],
  moistureThreshold: Number,
  lastPumped: Schema.Types.Date,
});

export default model<IWateringGroupModel>("WateringModel", WateringGroupSchema);
