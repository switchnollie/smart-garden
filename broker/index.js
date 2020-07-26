// Mongoose DB setup
const mongoose = require('mongoose');
const { initBroker } = require("./broker");

mongoose.Promise = global.Promise;

async function main() {
  await mongoose.connect(process.env.MONGO_URI, { useNewUrlParser: true });
  initBroker();
}

main();
