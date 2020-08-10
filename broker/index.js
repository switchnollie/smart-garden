// Mongoose DB setup
const mongoose = require("mongoose");
const { initBroker } = require("./broker");

mongoose.Promise = global.Promise;

async function main() {
  try {
    const db = await mongoose.connect(
      `${process.env.MONGO_URI}/${process.env.MONGO_DATABASE}`,
      {
        useNewUrlParser: true,
        useUnifiedTopology: true
      }
    );
    initBroker(db.connection);
  } catch (err) {
    console.error(err);
  }
}

main();
