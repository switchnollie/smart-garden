import { Router } from "express";

const router: Router = Router();

router.get("/", () => {
  // Get all users
});

router.post("/", () => {
  // Create a new user
});

router.get("/:id", () => {
  // Get User by ID
});

router.delete("/:id", () => {
  // Delete User by ID
});

export default router;
