import { Schema, model, Document } from "mongoose";
import { IWateringGroupModel } from "./wateringGroup";

export interface IDeviceModel extends Document {
  type: string;
  displayName: string;
  logBuckets: string[];
  groupedBy: IWateringGroupModel["_id"];
  lastValue: any;
  ownedBy: string;
}

const DeviceSchema = new Schema({
  type: String,
  displayName: String,
  lastValue: Schema.Types.Mixed,
  logBuckets: [
    {
      type: Schema.Types.ObjectId,
      ref: "DeviceLogBucket",
    },
  ],
  groupedBy: {
    type: Schema.Types.ObjectId,
    ref: "WateringGroup",
  },
  ownedBy: {
    type: Schema.Types.ObjectId,
    ref: "User",
  },
});

export default model<IDeviceModel>("Device", DeviceSchema);
