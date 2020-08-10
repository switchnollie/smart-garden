import { Request, Response } from "express";
import { Types } from "mongoose";
import UserModel, { IUserModel } from "../models/user";

const UserController = {
  async findAll(_: Request, res: Response): Promise<void> {
    try {
      const users = await UserModel.find({}).exec();
      console.log({ users });
      res.status(200).json(users);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async findOne(req: Request, res: Response): Promise<void> {
    try {
      const user = await UserModel.findOne({
        _id: Types.ObjectId(req.params.id),
      }).exec();

      if (user) {
        res.status(200).json(user);
      } else {
        res.status(404).send();
      }
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async create(req: Request, res: Response): Promise<void> {
    try {
      const user: IUserModel = await UserModel.create(req.body);

      res.status(201).json(user);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async updateOne(req: Request, res: Response): Promise<void> {
    try {
      const user: IUserModel = await UserModel.updateOne(
        { _id: req.params.id },
        req.body
      );

      res.status(200).json(user);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async remove(req: Request, res: Response): Promise<void> {
    try {
      const user = await UserModel.findOneAndRemove({
        _id: Types.ObjectId(req.params.id),
      }).exec();
      res.status(204).json(user);
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },
};

export default UserController;
