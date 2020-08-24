import mqtt from "mqtt";
const MQTT_PORT = process.env.MQTT_PORT || 1883;
const MQTT_HOST = process.env.MQTT_HOST || "localhost";

export default function connectToBroker() {
  console.log(`[MQTTClient] Connecting to mqtt://${MQTT_HOST}:${MQTT_PORT}`);
  const client = mqtt.connect(`mqtt://${MQTT_HOST}:${MQTT_PORT}`, {
    clientId: "admin-api",
  });

  client.on("connect", () => {
    console.log("[MQTTClient] connected");
  });

  return client;
}
