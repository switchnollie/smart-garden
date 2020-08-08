import { Schema, model, Document } from "mongoose";

export interface IUserModel extends Document {
  userName: string;
}

const UserSchema = new Schema({
  userName: String,
});

export default model<IUserModel>("User", UserSchema);
