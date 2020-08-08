import { disconnect, createConnection } from "mongoose";

export default function connectToMongo() {
  const connection = createConnection(
    `${process.env.MONGO_URI}/${process.env.MONGO_DATABASE}`,
    {
      useNewUrlParser: true,
      useUnifiedTopology: true,
    }
  );
  connection.on("connecting", () => {
    console.info("[MongoDB] connecting");
  });

  connection.on("connected", () => {
    console.log("[MongoDB] Connected");
  });

  connection.on("error", (error) => {
    console.error(`[MongoDB] connection ${error}`);
    disconnect();
  });

  connection.on("disconnected", () => {
    console.warn("[MongoDB] disconnected");
  });

  connection.on("reconnectFailed", () => {
    console.error("[MongoDB] reconnectFailed");
  });
  return connection;
}
