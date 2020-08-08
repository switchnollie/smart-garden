import { Router } from "express";
import UserController from "../controllers/user";

const router: Router = Router();

router.get("/", UserController.findAll);

router.post("/", UserController.create);

router.get("/:id", UserController.findOne);

router.delete("/:id", UserController.remove);

export default router;
