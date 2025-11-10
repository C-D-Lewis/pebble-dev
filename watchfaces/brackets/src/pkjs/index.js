var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

const API_URL = 'https://api.open-meteo.com/v1/forecast';

function request(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
  console.log('request(): ' + url);
}

function getWeatherStrFromCode(code) {
  // Always be 5 chars in length
  switch (parseInt(code, 10)) {
    case 0:
      return "CLEAR";
    case 1: // Mainly clear
    case 2: // Partly cloudy
    case 3: // Overcast
      return "CLOUD";
    // Fog and Rime Fog (45, 48)
    case 45: // Fog
    case 48: // Depositing rime fog
      return "FOG  ";
    // Drizzle (51, 53, 55) & Freezing Drizzle (56, 57)
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
      return "DRIZL";
    // Rain (61, 63, 65) & Freezing Rain (66, 67)
    case 61: // Slight Rain
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 66: // Light Freezing Rain
    case 67: // Heavy Freezing Rain
      return " RAIN";
    // Snow Fall and Snow Grains (71, 73, 75, 77)
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return " SNOW";
    // Rain Showers (80, 81, 82)
    case 80: // Slight Rain showers
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
      return "SHOWR";
    // Thunderstorm (95, 96, 99)
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return "THNDR";
    // Default/Unknown Code
    default:
      return "?    ";
  }
}

function stringRepeat(str, count) {
  if (count < 0) throw new RangeError("repeat count must be non-negative.");

  var result = '';
  for (var i = 0; i < count; i++) {
    result += str;
  }
  return result;
}

function getWeather(pos) {
  var params = [
    'latitude=' + pos.coords.latitude,
    'longitude=' + pos.coords.longitude,
    'current=temperature_2m,weather_code',
    'temperature_unit=celsius'  // TODO: Configurable
  ].join('&');

  var url = API_URL + '?' + params;
  request(url, 'GET', (responseText) => {
    var json = JSON.parse(responseText);
    console.log(JSON.stringify(json, null, 2));

    var temp = parseInt(json.current.temperature_2m, 10);
    var tempStr = (temp < 10 ? ' ' : '') + temp;
    var code = json.current.weather_code;

    var str = getWeatherStrFromCode(code);
    var gap = 8 - (tempStr.length + str.length);
    var weatherStr = tempStr + stringRepeat(' ', gap) + str;

    Pebble.sendAppMessage({
      WeatherString: weatherStr,
    }, function() {
      console.log('Sent weather to Pebble');
    }, console.log);
  }); 
}

function getLocation() {
  window.navigator.geolocation.getCurrentPosition(
    getWeather,
    function (err) {
      console.log(err);
      Pebble.sendAppMessage({'KEY_REQUEST_TEMPERATURE': 'ERR'});
    },
    {'timeout': 15000, 'maximumAge': 60000}
  );
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  getLocation();
});

Pebble.addEventListener('appmessage', function() {
  getLocation();
});
