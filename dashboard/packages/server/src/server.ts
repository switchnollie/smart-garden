import express from "express";
import logger from "morgan";
import cors from "cors";
import path from "path";
import initRoutes from "./routes";
import connectToMongo from "./dbConnection";

const app = express();
const PORT = 4000;

app.use(cors());
app.use(logger("dev"));
app.use(express.json());

app.use(express.static(path.join(__dirname, "public")));

app.get("/api", (_, res) => {
  res.send("API is working properly");
});

app.listen(PORT, () => {
  console.log(`Dashboard started an listening on port ${PORT}`);
});

initRoutes(app);
connectToMongo();

export default app;
