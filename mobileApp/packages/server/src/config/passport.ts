import path from "path";
import fs from "fs";
import { PassportStatic } from "passport";
import {
  ExtractJwt,
  Strategy as JWTStrategy,
  StrategyOptions,
  VerifyCallback,
} from "passport-jwt";
import User from "../models/user";

const pathToKey = path.join(__dirname, "..", "..", "id_rsa_pub.pem");
const PUB_KEY = fs.readFileSync(pathToKey, "utf8");

const passportOptions: StrategyOptions = {
  jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
  secretOrKey: PUB_KEY,
  algorithms: ["RS256"],
};

const verifyRequest: VerifyCallback = async (payload, done) => {
  try {
    // JWT is valid, lookup for the user that is specified in the sub field
    const user = await User.findOne({ _id: payload.sub });
    if (user) {
      // call done so passport can attach the user to `request.user`
      return done(null, user);
    } else {
      return done(null, false);
    }
  } catch (err) {
    done(err, null);
  }
};

const strategy = new JWTStrategy(passportOptions, verifyRequest);

export default function configurePassport(passport: PassportStatic) {
  passport.use(strategy);
}
