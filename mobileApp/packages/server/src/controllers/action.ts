import { Request, Response } from "express";
import { MqttClient } from "mqtt";
import Device from "../models/device";

const ActionController = {
  async triggerPump(req: Request, res: Response) {
    const { userId, groupId } = req.body;
    if (userId && groupId) {
      try {
        // Get all pumps for group
        const pumpIds = (
          await Device.find({ type: "pump", groupedBy: groupId }, "_id").exec()
        ).map((doc) => doc._id as string);
        // publish on the pump topic for every pump in the group
        console.log({ pumpIds, userId, groupId });
        console.log({ mqttClient: req.app.locals.mqttClient });
        pumpIds.forEach((pumpId: string) => {
          (req.app.locals.mqttClient as MqttClient).publish(
            `${userId}/${groupId}/${pumpId}/pump`,
            "1"
          );
        });

        res.status(200).send();
      } catch (error) {
        console.error(error);
        res.sendStatus(500);
      }
    } else {
      res.sendStatus(400);
    }
  },
};

export default ActionController;
