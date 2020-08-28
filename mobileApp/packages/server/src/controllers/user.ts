import { Request, Response } from "express";
import { Types } from "mongoose";
import User, { IUserModel } from "../models/user";
import { genPassword, validPassword, issueJWT } from "../utils/passwordUtils";

const UserController = {
  async login(req: Request, res: Response): Promise<void> {
    try {
      const user = await User.findOne({ userName: req.body.username });
      if (!user) {
        res.status(401).json({ success: false, msg: "could not find user" });
      }
      const isValid = validPassword(req.body.password, user!.hash, user!.salt);

      if (isValid) {
        const { token, expires } = issueJWT(user!);
        res
          .status(200)
          .json({ success: true, user, token, expiresIn: expires });
      } else {
        res
          .status(401)
          .json({ success: false, msg: "you entered an incorrect pasword" });
      }
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },

  async register(req: Request, res: Response): Promise<void> {
    const { salt, hash } = genPassword(req.body.password);

    try {
      const existingUser = await User.findOne({userName: req.body.username});
      if (!!existingUser) {
        res.status(409).send();
      } else {
        const newUser = new User({
          userName: req.body.username,
          hash,
          salt,
        });
        const user = await newUser.save();
        const { token, expires } = issueJWT(user);
        res.json({ success: true, user, token, expiresIn: expires });
      }
    } catch (error) {
      console.error(error);
      res.status(400).send();
    }
  },
};

export default UserController;
