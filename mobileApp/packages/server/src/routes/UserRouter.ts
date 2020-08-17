import { Router } from "express";
import UserController from "../controllers/user";

const router: Router = Router();

router.post("/login", UserController.login);

router.post("/register", UserController.register);

export default router;
