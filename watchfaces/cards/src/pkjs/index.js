var secrets = require('./secrets.js');

/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

var getValue = function(dict, key)  {
  if(hasKey(dict, key)) {
    return '' + dict.payload[key];
  } else {
    console.log('ERROR: Key' + key + ' does not exist in received dictionary');
    return undefined;
  }
};

/*************** Requests **********************/

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

/****************** Weather ********************/

var lastConditions = '';
var lastLocation = '';

function locationSuccess(pos) {
  var url = 'http://api.weatherapi.com/v1/current.json?key=' + secrets.token + '&q=' + pos.coords.latitude + ',' + pos.coords.longitude;
  console.log('Getting weather from: ' + url);

  xhrRequest(url, 'GET', function(responseText) {
    var json = JSON.parse(responseText);
    console.log(JSON.stringify(json));

    var location = json.location.name;
    var conditions = json.current.condition.text;
    //  var temperature = Math.round(json.main.temp - 273.15);

    // Need to update?
    if (lastLocation != location || lastConditions != conditions) {
      lastLocation = location;
      lastConditions = conditions;

      var dict = {
        'KEY_LOCATION': location,
        'KEY_CONDITIONS': conditions,
      };
      console.log('Dict: ' + JSON.stringify(dict));

      // Send
      Pebble.sendAppMessage(dict, function(e) {
        console.log('Weather fetch complete!');
      }, function(e) { 
        console.log('Error sending weather to watch');
        console.log(JSON.stringify(e));
      });
    }
  });
}

function getWeather() {
  window.navigator.geolocation.getCurrentPosition(
    locationSuccess,
    function (err) {
      console.log(err);
      Pebble.sendAppMessage({'KEY_REQUEST_TEMPERATURE': 'ERR'});
    },
    {'timeout': 15000, 'maximumAge': 60000}
  );
}

/******************** App Lifecycle ********************/

Pebble.addEventListener('ready', function(e) {
  console.log('JS Ready!');
  getWeather();
});

Pebble.addEventListener('appmessage', function(e) {
  getWeather();
});
