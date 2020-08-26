import { Request, Response } from "express";
import { Types } from "mongoose";
import WateringGroupModel, {
  IWateringGroupModel,
} from "../models/wateringGroup";
import DeviceModel, { IDeviceModel } from "../models/device";
import UserModel from "../models/user";

const WateringGroupController = {
  async findAll(_: Request, res: Response): Promise<void> {
    try {
      const wateringGroups = await WateringGroupModel.find({}, "-__v")
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
      console.log({ devicesInfo, rest });
      if (Array.isArray(devicesInfo) && devicesInfo.length > 0) {
        // Add Devices
        await Promise.all(
          devicesInfo.map((device) => DeviceModel.create(device))
        );
        const deviceIds = devicesInfo.map((device) => device._id);
        // Add Watering Group
        const wateringGroup: IWateringGroupModel = await WateringGroupModel.create(
          { devices: deviceIds, ...rest }
        );
        console.log({ wateringGroup });
        // Add Wateringgroup to user.
        await UserModel.updateOne(
          { _id: wateringGroup.ownedBy },
          { $push: { wateringGroups: wateringGroup._id } }
        );

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
