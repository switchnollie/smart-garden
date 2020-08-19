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

app.use(express.static(path.join(__dirname, "..", "public")));

initRoutes(app);

// Handles any requests that don't match the ones above
app.get("*", (req, res) => {
  res.sendFile(path.join(__dirname, "..", "public", "index.html"));
});

app.listen(PORT, () => {
  console.log(`Server started and listening on port ${PORT}`);
});

export default app;
