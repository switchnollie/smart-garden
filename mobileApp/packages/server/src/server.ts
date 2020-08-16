import express from "express";
import logger from "morgan";
import cors from "cors";
import path from "path";
import passport from "passport";
import initRoutes from "./routes";
import connectToMongo from "./config/dbConnection";
import connectToBroker from "./config/mqttConnection";
import configurePassport from "./config/passport";

const app = express();
const PORT = 4000;

app.use(cors());
app.use(logger("dev"));
app.use(express.json());

// Setup Authentication with JWT
configurePassport(passport);
app.use(passport.initialize());

app.locals.db = connectToMongo();
app.locals.mqttClient = connectToBroker();

app.use(express.static(path.join(__dirname, "public")));

app.listen(PORT, () => {
  console.log(`Server started and listening on port ${PORT}`);
});

initRoutes(app);

export default app;
