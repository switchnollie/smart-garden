const express = require("express");
const logger = require("morgan");
const cors = require("cors");
const path = require("path");

const app = express();
const PORT = 4000;

app.use(cors());
app.use(logger("dev"));
app.use(express.json());

app.use(express.static(path.join(__dirname, "public")));

app.get("/api", (_, res,) => {
  res.send("API is working properly");
});

app.listen(PORT, () => {
  console.log(`Dashboard started an listening on port ${PORT}`);
});

