import { connect } from "mongoose";

export default async function connectToMongo() {
  return await connect(
    `${process.env.MONGO_URI}/${process.env.MONGO_DATABASE}`,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
    }
  );
}
