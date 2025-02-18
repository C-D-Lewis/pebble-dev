const USER = process.env.USER || 'pi';
const path = `/home/${USER}/code/node-microservices`;

/**
 * Import modules directly from the node-common project in this repo.
 *
 * @param {string[]} list - Array of modules names.
 * @returns {object[]} Array of loaded modules.
 */
module.exports = (list) => list.reduce((acc, item) => ({
  ...acc,
  [item]: require(`${path}/node-common/src/modules/${item}`),
}), {});
