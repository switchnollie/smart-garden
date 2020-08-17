import mqtt from "mqtt";
const MQTT_PORT = process.env.MQTT_PORT || 1883;

export default function connectToBroker() {
  console.log(`[MQTTClient] Connecting to mqtt://localhost:${MQTT_PORT}`);
  const client = mqtt.connect(`mqtt://localhost:${MQTT_PORT}`, {
    clientId: "admin-api",
  });

  client.on("connect", () => {
    console.log("[MQTTClient] connected");
  });

  return client;
}
