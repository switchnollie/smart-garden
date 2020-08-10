import { Router } from "express";
import WateringGroupController from "../controllers/wateringGroup";

const router: Router = Router();

router.get("/", WateringGroupController.findAll);

router.post("/", WateringGroupController.create);

router.get("/:id", WateringGroupController.findOne);

router.put("/:id", WateringGroupController.updateOne);

router.delete("/:id", WateringGroupController.remove);

export default router;
