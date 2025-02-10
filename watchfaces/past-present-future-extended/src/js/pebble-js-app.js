/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== "undefined";
};

var getValue = function(dict, key) {
  if(hasKey(dict, key)) {
    return "" + dict.payload[key];
  } else {
    console.log("ERROR: Key '" + key + "' does not exist in received dictionary");
    return undefined;
  }
};

/*************** Requests **********************/

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload =  function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

/****************** Weather ********************/

function locationSuccess(pos) {
  xhrRequest(
		"http://api.openweathermap.org/data/2.5/weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude,
		"GET",
		function(responseText) {
			var json = JSON.parse(responseText);
			console.log(JSON.stringify(json));

      var temperature = Math.round(json.main.temp - 273.15);

      Pebble.sendAppMessage(
        {"KEY_REQUEST_TEMPERATURE": "" + temperature},
        function(e) {
          console.log("Weather fetch complete!");
        },
        function(e) { }
      );
		}
	);
}

function locationError(err) {
  Pebble.sendAppMessage({"KEY_REQUEST_TEMPERATURE": "ERR"});
}

function getWeather() {
  window.navigator.geolocation.getCurrentPosition(
		locationSuccess,
	  locationError,
	  {"timeout": 15000, "maximumAge": 60000}
	);
}

/**************** App Main *********************/

Pebble.addEventListener("ready",
  function(e) {
    getWeather();
  }
);

Pebble.addEventListener("appmessage",
  function(dict) {
    if(hasKey(dict, "KEY_REQUEST_TEMPERATURE")) {
      getWeather();
    }
  }
);