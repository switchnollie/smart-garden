import { disconnect, connect } from "mongoose";

export default async function connectToMongo() {
  const db = await connect(
    `${process.env.MONGO_URI}/${process.env.MONGO_DATABASE}`,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
    }
  );

  db.connection.on("connecting", () => {
    console.info("[MongoDB] connecting");
  });

  db.connection.on("connected", () => {
    console.log("[MongoDB] Connected");
  });

  db.connection.on("error", (error) => {
    console.error(`[MongoDB] connection ${error}`);
    disconnect();
  });

  db.connection.on("disconnected", () => {
    console.warn("[MongoDB] disconnected");
  });

  db.connection.on("reconnectFailed", () => {
    console.error("[MongoDB] reconnectFailed");
  });
  return db;
}
