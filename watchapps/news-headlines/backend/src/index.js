require('dotenv').config();

const data = require('./modules/data');

const { UPDATE_INTERVAL_M } = process.env;

(async () => {
  setInterval(data.download, UPDATE_INTERVAL_M * 1000 * 60);
  data.download();
})();
