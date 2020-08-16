import { Router } from "express";
import ActionController from "../controllers/action";

const router: Router = Router();

router.get("/:groupId/:deviceId/pump", ActionController.triggerPump);

export default router;
