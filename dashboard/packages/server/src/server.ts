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

connectToMongo();

app.use(express.static(path.join(__dirname, "public")));

app.listen(PORT, () => {
  console.log(`Dashboard started and listening on port ${PORT}`);
});

initRoutes(app);

export default app;
