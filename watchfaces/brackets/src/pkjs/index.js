var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});
