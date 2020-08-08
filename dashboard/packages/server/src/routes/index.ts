import { Application } from "express";
import UserRouter from "./UserRouter";

export default function initRoutes(app: Application): void {
  app.use("/api/users", UserRouter);
}
