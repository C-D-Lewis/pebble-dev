import { setupClay } from './clay.js';
import { WeatherApiResponse } from './types.js';

/** Signed offset to allow negative numbers */
const SIGNED_OFFSET = 50;

/** Disable before release! */
const TEST_MODE = false;

/** Sample temperate temperatures */
const TEST_TEMP_ARR = [4, 3, 3, 3, 5, 7, 8, 5, 5, 6, 8, 13, 15, 15, 14, 13, 7, 7, 8, 7, 7, 7, 5, 5];
/** Sample precipitation data */
const TEST_PRECIP_ARR = [0, 0, 0, 0, 0, 0, 50, 60, 60, 30, 20, 0, 0, 0, 0, 40, 80, 99, 99, 50, 0, 0, 0, 0];
/** Test position */
const TEST_POSITION = { coords: { latitude: 51.74, longitude: 0.48 } };
// Birmingham
// const TEST_POSITION = { coords: { latitude: 52.489471, longitude: -1.898575 } };

setupClay();

/**
 * Zero pad a number, max 99
 */
const zeroPad = (num: number) => {
  if (num < 10) return `0${num}`;
  if (num > 99) return '99';
  return String(num);
};

/**
 * Encode number to two digits, adding 50 to handle negative values, which
 * will be subtracted on the watch.
 */
const encodeSignedNumber = (num: number) => {
  const val = Math.min(Math.max(Math.round(num) + SIGNED_OFFSET, 0), 99);
  return val >= 10 ? String(val) : `0${val}`;
};

/**
 * Fetch weather for this day from OpenMeteo.
 */
const fetchWeatherForecast = async (lat: number, lon: number ): Promise<WeatherApiResponse> => {
  const params = {
    latitude: lat,
    longitude: lon,
    hourly: 'temperature_2m,weather_code,precipitation_probability',
    current: 'temperature_2m,weather_code,relative_humidity_2m,wind_speed_10m',
    daily: 'sunrise,sunset',
    forecast_days: 1,
    temperature_unit: 'celsius',
    timezone: 'auto',
  };
  const paramStr = Object.entries(params).map(([k, v]) => `${k}=${v}`).join('&'); 
  const url = `https://api.open-meteo.com/v1/forecast?${paramStr}`;

  const res = await fetch(url);
  if (res.status !== 200) throw new Error(await res.text());
  
  const json = await res.json() as WeatherApiResponse;
  // console.log(JSON.stringify(json, null, 2));
  return json;
};

/**
 * Get current rough location.
 */
const getLocation = async () => new Promise((resolve, reject) => {
  navigator.geolocation.getCurrentPosition(
    resolve,
    reject,
    { timeout: 5000, maximumAge: 60000 },
  );
});

/**
 * Get the time portion of ISO datetime.
 */
const getTime = (str: string) => str.split('T')[1];

/**
 * Get location and weather, and send to watch.
 */
const sendWeather = async () => {
  try {
    const pos = TEST_MODE ? TEST_POSITION : (await getLocation() as GeolocationPosition);
    const { latitude, longitude } = pos.coords;
    console.log(`Got location: ${latitude}, ${longitude}`);

    const { current, hourly, daily } = await fetchWeatherForecast(latitude, longitude);
    const {
      temperature_2m: currentTemp,
      weather_code: currentCode,
      relative_humidity_2m: currentHumidity,
      wind_speed_10m: currentWindSpeed,
    } = current;
    const {
      time,
      temperature_2m: hourlyTemps,
      weather_code: hourlyCodes,
      precipitation_probability: hourlyPrecip,
    } = hourly;
    const { sunrise, sunset } = daily;
    console.log(`Time range: ${time[0]} - ${time[time.length - 1]}`);

    const tempArr = TEST_MODE ? TEST_TEMP_ARR : hourlyTemps;
    const precipArr = TEST_MODE ? TEST_PRECIP_ARR : hourlyPrecip;

    const sunriseTime = getTime(sunrise[0]);
    const sunsetTime = getTime(sunset[0]);

    /**
     * Arrays are two chars per item, 24 items:
     *   "CODES": "030303030303030302030202030202030302000100000000"
     */
    const dict = {
      CURRENT_TEMP: Math.round(currentTemp),
      CURRENT_CODE: currentCode,
      SUNRISE: sunriseTime,
      SUNSET: sunsetTime,
      CURRENT_HUMIDITY: currentHumidity,
      CURRENT_WIND: currentWindSpeed,
      TEMP_ARR: tempArr.map(encodeSignedNumber).join(''),
      PRECIP_ARR: precipArr.map(zeroPad).join(''),
      CODE_ARR: hourlyCodes.map(zeroPad).join(''),
    };
    console.log(JSON.stringify(dict, null, 2));
    
    // Send to watch
    await PebbleTS.sendAppMessage(dict);
    console.log('Weather data sent to watch');
  } catch (e) {
    console.log('Error fetching weather');
    console.log(e);

    await PebbleTS.sendAppMessage({ WEATHER_ERROR: 1 });
  }
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  sendWeather();
});

Pebble.addEventListener('appmessage', async (e) => {
  const { payload: dict } = e;
  console.log(`appmessage: ${JSON.stringify(dict)}`);

  sendWeather();
});
