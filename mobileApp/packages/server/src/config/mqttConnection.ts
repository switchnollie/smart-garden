import mqtt from "mqtt";
const MQTT_PORT = process.env.MQTT_PORT || 1883;

export default function connectToBroker() {
  const client = mqtt.connect(`localhost:${MQTT_PORT}`, {
    clientId: "admin-api",
  });

  client.on("connect", () => {
    console.log("[MQTTClient] connected");
  });

  return client;
}
