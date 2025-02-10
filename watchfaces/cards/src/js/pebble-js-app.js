/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

var getValue = function(dict, key) {
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

var 
  lastConditions = '',
  lastLocation = '';

function locationSuccess(pos) {
	console.log('Getting weather from: ' + 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude);

  xhrRequest(
		'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude,
		'GET',
		function(responseText) {
			var json = JSON.parse(responseText);
			console.log(JSON.stringify(json));

      //Location
      var location = json.name;

      //Conditions - always upper case first letter
      var conditions = json.weather[0].description;
      conditions = conditions.charAt(0).toUpperCase() + conditions.substring(1);

      //Temperature
      var temperature = Math.round(json.main.temp - 273.15);

      //Need to update?
      if(lastLocation != location || lastConditions != conditions) {
        //Remember new values
        lastLocation = location;
        lastConditions = conditions;

        //Assemble dictionary
        var dict = {
        	'KEY_LOCATION': location,
        	'KEY_CONDITIONS': conditions,
        };
        console.log('Dict: ' + JSON.stringify(dict));

        //Send
        Pebble.sendAppMessage(
          dict,
          function(e) {
            console.log('Weather fetch complete!');
          },
          function(e) { 
            console.log('Error sending weather to watch');
          }
        );
      }
		}
	);
}

function locationError(err) {
  Pebble.sendAppMessage({'KEY_REQUEST_TEMPERATURE': 'ERR'});
}

function getWeather() {
  window.navigator.geolocation.getCurrentPosition(
		locationSuccess,
	  locationError,
	  {'timeout': 15000, 'maximumAge': 60000}
	);
}

/******************** App Lifecycle ********************/
Pebble.addEventListener('ready', 
	function(e) {
		console.log('JS Ready!');
		getWeather();
    getAppVersion();
	}
);

Pebble.addEventListener('appmessage',
  function(e) {
    getWeather();
  }
);