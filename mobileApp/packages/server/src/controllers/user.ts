import { Request, Response } from "express";
import { Types } from "mongoose";
import UserModel, { IUserModel } from "../models/user";

const UserController = {
  async login(_: Request, res: Response): Promise<void> {
    // try {
    //   const users = await UserModel.find({}).exec();
    //   console.log({ users });
    //   res.status(200).json(users);
    // } catch (error) {
    //   console.error(error);
    //   res.status(400).send();
    // }
  },

  async register(req: Request, res: Response): Promise<void> {
    // try {
    //   const user = await UserModel.findOne({
    //     _id: Types.ObjectId(req.params.id),
    //   }).exec();
    //   if (user) {
    //     res.status(200).json(user);
    //   } else {
    //     res.status(404).send();
    //   }
    // } catch (error) {
    //   console.error(error);
    //   res.status(400).send();
    // }
  },
};

export default UserController;
