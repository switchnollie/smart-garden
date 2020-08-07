module.exports = function () {
  const currentDate = new Date();
  const date =
    currentDate.getFullYear() +
    "-" +
    (currentDate.getMonth() + 1) +
    "-" +
    currentDate.getDate();

  return new Date(date);
};
