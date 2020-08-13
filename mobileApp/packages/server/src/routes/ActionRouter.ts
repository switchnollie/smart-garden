import { Router } from "express";
import ActionController from "../controllers/action";

const router: Router = Router();

router.get("/pump/:id", ActionController.triggerPump);

export default router;
