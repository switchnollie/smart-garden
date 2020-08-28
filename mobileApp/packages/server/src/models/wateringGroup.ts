import { Schema, model, Document } from "mongoose";
import { IDeviceModel } from "./device";

export interface IWateringGroupModel extends Document {
  displayName: string;
  ownedBy: string;
  devices: IDeviceModel["_id"][];
  moistureThreshold: number;
  lastPumped: Date;
}

const WateringGroupSchema = new Schema({
  displayName: String,
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "user",
  },
  devices: [
    {
      type: String,
      ref: "device",
    },
  ],
  moistureThreshold: {type: Number, default: 411},
  lastPumped: Schema.Types.Date,
  minimalPumpInterval: {type: Number, default: 86400000},
});

export default model<IWateringGroupModel>("WateringGroup", WateringGroupSchema);
