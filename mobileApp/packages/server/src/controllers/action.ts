import { Request, Response } from "express";
import { MqttClient } from "mqtt";

const ActionController = {
  async triggerPump(req: Request, res: Response) {
    (req.app.locals.mqttClient as MqttClient).publish("test", "test123");
  },
};

export default ActionController;
