// The "Mongo-way" of designing time series data schemas
// called "Size-based bucketing" is applied here
// https://www.mongodb.com/blog/post/time-series-data-and-mongodb-part-2-schema-design-best-practices
const { Schema, model } = require("mongoose");

const DeviceLogBucketSchema = new Schema(
  {
    deviceId: {
      type: String,
      ref: "Device"
    },
    nsamples: Number,
    day: Schema.Types.Date,
    first: Number, // Unix Timestamp
    last: Number, // Unix Timestamp
    samples: [
      {
        time: Number
      }
    ]
  },
  { strict: false }
);

module.exports = model("DeviceLogBucket", DeviceLogBucketSchema);
