require('dotenv').config();

const data = require('./modules/data');

const { UPDATE_INTERVAL_M } = process.env;

/**
 * Main function.
 */
const main = () => {
  if (!UPDATE_INTERVAL_M) throw new Error('UPDATE_INTERVAL_M is not set in .env');

  setInterval(data.download, UPDATE_INTERVAL_M * 1000 * 60);
  data.download();
};

main();
