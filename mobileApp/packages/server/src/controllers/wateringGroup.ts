import { Request, Response } from "express";
import { Types } from "mongoose";
import WateringGroupModel, {
  IWateringGroupModel,
} from "../models/wateringGroup";
import DeviceModel, { IDeviceModel } from "../models/device";
import UserModel from "../models/user";

const WateringGroupController = {
  async findAll(req: Request, res: Response): Promise<void> {
    try {
      console.log((req.user! as any)._id);
      const wateringGroups = await WateringGroupModel.find(
        { ownedBy: (req.user! as any)._id },
        "-__v"
      )
        .populate({
          path: "devices",
          select: "-__v -groupedBy",
          model: DeviceModel,
        })
        .exec();
      res.status(200).json(wateringGroups);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async findOne(req: Request, res: Response): Promise<void> {
    try {
      const wateringGroup = await WateringGroupModel.findOne(
        {
          _id: Types.ObjectId(req.params.id),
        },
        "-__v"
      )
        .populate({
          path: "devices",
          select: "-__v -groupedBy",
          model: DeviceModel,
        })
        .exec();
      if (wateringGroup) {
        res.status(200).json(wateringGroup);
      } else {
        res.status(404).send();
      }
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async updateOne(req: Request, res: Response): Promise<void> {
    try {
      const wateringGroup: IWateringGroupModel = await WateringGroupModel.updateOne(
        { _id: req.params.id },
        req.body
      );

      res.status(200).json(wateringGroup);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async create(req: Request, res: Response): Promise<void> {
    try {
      const { devices: devicesInfo, ...rest } = req.body;
      if (Array.isArray(devicesInfo) && devicesInfo.length > 0) {
        // Add Devices
        await Promise.all(
          devicesInfo.map((device) => DeviceModel.create(device))
        );
        const deviceIds: string[] = devicesInfo.map((device) => device._id);
        // If a wateringGroup with the provided name is already created, just add the devices to the group
        const existingWateringGroup = await WateringGroupModel.findOne({
          displayName: rest.displayName,
        });
        let wateringGroup: IWateringGroupModel;
        if (!!existingWateringGroup) {
          wateringGroup = await WateringGroupModel.findByIdAndUpdate(existingWateringGroup._id,
            // @ts-ignore
            { $push: { devices: deviceIds } },
            {new: true}
          );
          console.log({ wateringGroup });
        } else {
          // Add Watering Group
          wateringGroup = await WateringGroupModel.create({
            devices: deviceIds,
            ...rest,
          });
          console.log({ wateringGroup });
          // Add Wateringgroup to user.
          await UserModel.updateOne(
            { _id: wateringGroup.ownedBy },
            { $push: { wateringGroups: wateringGroup._id } }
          );
        }

        res.status(201).json(wateringGroup);
      }
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async remove(req: Request, res: Response): Promise<void> {
    try {
      const wateringGroup = await WateringGroupModel.findOneAndRemove({
        _id: Types.ObjectId(req.params.id),
      }).exec();
      res.status(204).json(wateringGroup);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },
};

export default WateringGroupController;
