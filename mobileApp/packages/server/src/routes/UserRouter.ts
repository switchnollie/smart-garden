import { Router } from "express";
import UserController from "../controllers/user";

const router: Router = Router();

router.post("/", UserController.login);

router.post("/", UserController.register);

export default router;
