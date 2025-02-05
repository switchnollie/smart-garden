import { Application } from "express";
import UserRouter from "./UserRouter";
import WateringGroupRouter from "./WateringGroupRouter";
import ActionRouter from "./ActionRouter";

export default function initRoutes(app: Application): void {
  app.use("/api/user", UserRouter);
  app.use("/api/wateringgroup", WateringGroupRouter);
  app.use("/api/action", ActionRouter);
}
