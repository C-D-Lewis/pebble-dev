import { setupClay } from './clay.js';

setupClay();

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');
});
