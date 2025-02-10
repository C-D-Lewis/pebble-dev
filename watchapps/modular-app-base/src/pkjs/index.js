var VERSION = '1.0';
var DEBUG = false;

/********************************** Helpers ***********************************/

function info(content) {
  console.log(content);
}

function debug(content) {
  if(DEBUG) info(content);
}

function request(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

/******************************** PebbleKit JS ********************************/

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready! Version ' + VERSION);
});

Pebble.addEventListener('appmessage', function(data) {
  var dict = data.payload;
  debug('Got appmessage: ' + JSON.stringify(dict));
});
