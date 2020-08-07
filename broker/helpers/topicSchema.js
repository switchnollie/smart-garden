function parseTopic(topic) {
  const topicParts = topic.split("/");
  if (topicParts.length < 4) {
    return {
      length: topicParts.length,
      deviceId: null,
      userId: null,
      groupId: null,
      type: null
    };
  }
  return {
    length: topicParts.length,
    userId: topicParts[0],
    groupId: topicParts[1],
    deviceId: topicParts[2],
    type: topicParts[3]
  };
}

module.exports = { parseTopic };
