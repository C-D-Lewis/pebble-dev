var VERSION = '1.12';

var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready! Version: ' + VERSION);
});
