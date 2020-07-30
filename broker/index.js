// Mongoose DB setup
const mongoose = require('mongoose');
const { initBroker } = require("./broker");

mongoose.Promise = global.Promise;

async function main() {
  console.log({environment: process.env});
  try {
    await mongoose.connect(process.env.MONGO_URI, { useNewUrlParser: true, 
      useUnifiedTopology: true, });
  } catch (err) {
    console.error(err);
  }
  initBroker(mongoose.connection);
}

main();
