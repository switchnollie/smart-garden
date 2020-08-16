import { Router } from "express";
import WateringGroupController from "../controllers/wateringGroup";
import passport from "passport";

const router: Router = Router();

// router.use(passport.authenticate("jwt", { session: false }));

router.get("/", WateringGroupController.findAll);

router.post("/", WateringGroupController.create);

router.get("/:id", WateringGroupController.findOne);

router.put("/:id", WateringGroupController.updateOne);

router.delete("/:id", WateringGroupController.remove);

export default router;
