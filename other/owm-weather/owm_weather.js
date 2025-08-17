/*************************** Weather library start ****************************/

var owmWeatherAPIKey = '';

function owmWeatherXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function owmWeatherSendToPebble(json) {
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyReply': 1,
    'OWMWeatherAppMessageKeyDescription': json.weather[0].description,
    'OWMWeatherAppMessageKeyDescriptionShort': json.weather[0].main,
    'OWMWeatherAppMessageKeyTempK': json.main.temp,
    'OWMWeatherAppMessageKeyName': json.name
  });
}

function owmWeatherLocationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + 
    pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + owmWeatherAPIKey;
  console.log('owm-weather: Location success. Contacting OpenWeatherMap.org...');

  owmWeatherXHR(url, 'GET', function(responseText) {
    console.log('owm-weather: Got API response!');
    if(responseText.length > 100) {
      owmWeatherSendToPebble(JSON.parse(responseText));
    } else {
      console.log('owm-weather: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'OWMWeatherAppMessageKeyBadKey': 1
      });
    }
  });
}

function owmWeatherLocationError(err) {
  console.log('owm-weather: Location error');
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyLocationUnavailable': 1
  });
}

function owmWeatherHandler(dict) {
  if(dict.payload['OWMWeatherAppMessageKeyRequest']) {
    owmWeatherAPIKey = dict.payload['OWMWeatherAppMessageKeyRequest'];
    console.log('owm-weather: Got fetch request from C app');
    
    navigator.geolocation.getCurrentPosition(owmWeatherLocationSuccess, owmWeatherLocationError, {
      timeout: 15000,
      maximumAge: 60000
    });
  }
};

/**************************** Weather library end *****************************/