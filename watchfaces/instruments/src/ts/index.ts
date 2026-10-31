import { setupClay } from './clay.js';
import { WeatherApiResponse } from './types.js';

/** Disable before release! */
const TEST_MODE = false;

/** Test position */
const TEST_POSITION = { coords: { latitude: 51.74, longitude: 0.48 } };
/** Sample precipitation data */
const TEST_PRECIP_ARR = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 40, 20, 50, 90, 20];
/** Sample weather codes */
const TEST_CODES_ARR = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 77, 82, 99];

setupClay();

/**
 * Zero pad a number, max 99
 */
const zeroPadTo99 = (num: number) => {
  if (num < 10) return `0${num}`;
  if (num > 99) return '99';
  return String(num);
};

/**
 * Fetch weather for this day from OpenMeteo.
 */
const fetchWeatherForecast = async (lat: number, lon: number ): Promise<WeatherApiResponse> => {
  const params = {
    latitude: lat,
    longitude: lon,
    hourly: 'temperature_2m,weather_code,precipitation_probability',
    current: 'temperature_2m,weather_code',
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
    { timeout: 30000, maximumAge: 60000 },
  );
});

/**
 * Get the time portion of ISO datetime in format HH:MM, e.g. 06:12
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

    const { current, daily, hourly } = await fetchWeatherForecast(latitude, longitude);
    const {
      temperature_2m: currentTemp,
      weather_code: currentCode,
    } = current;
    const { sunrise, sunset } = daily;
    const {
      precipitation_probability: hourlyPrecip,
      weather_code: hourlyCodes,
      temperature_2m: hourlyTemps,
    } = hourly;

    const sunriseTime = getTime(sunrise[0]);
    const sunsetTime = getTime(sunset[0]);

    // Need either the next 6, or the last six of the day
    const precipArr = TEST_MODE ? TEST_PRECIP_ARR : hourlyPrecip;
    const startHour = Math.min(new Date().getHours(), precipArr.length - 6);
    const nextSixPrecip = precipArr.slice(startHour, startHour + 6);
    const codesArr = TEST_MODE ? TEST_CODES_ARR : hourlyCodes;
    const nextSixCodes = codesArr.slice(startHour, startHour + 6);

    // Derive low/high from array so it never exceeds
    let tempLow = 999;
    let tempHigh = -999;
    hourlyTemps.forEach((i) => {
      if (i < tempLow) {
        tempLow = i;
      }
      if (i > tempHigh) {
        tempHigh = i;
      }
    });

    const dict = {
      CURRENT_TEMP: Math.round(currentTemp),
      CURRENT_CODE: currentCode,
      SUNRISE: sunriseTime,
      SUNSET: sunsetTime,
      PRECIP_ARR: nextSixPrecip.map(zeroPadTo99).join(''),
      CODE_ARR: nextSixCodes.map(zeroPadTo99).join(''),
      TEMP_LOW: Math.round(tempLow),
      TEMP_HIGH: Math.round(tempHigh),
    };
    console.log(JSON.stringify(dict, null, 2));
    
    // Send to watch
    await PebbleTS.sendAppMessage(dict);
    console.log('Weather data sent to watch');
  } catch (err: unknown) {
    console.log('Error fetching weather');
    console.log(err);

    if (err instanceof Error) {
      console.log('Error string: ' + JSON.stringify(err));
    } else {
      const e = err as GeolocationPositionError;
      if (e.code || e.message) {
        console.log('Error Code: ' + e.code);
        console.log('Error Msg: ' + e.message);
      }
    }

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
