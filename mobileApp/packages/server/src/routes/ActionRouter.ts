import { Router } from "express";
import ActionController from "../controllers/action";
import passport from "passport";

const router: Router = Router();

// router.use(passport.authenticate("jwt", { session: false }));

router.get("/:groupId/:deviceId/pump", ActionController.triggerPump);

export default router;
