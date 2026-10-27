import { setupClay } from './clay.js';
import { WeatherApiResponse } from './types.js';

/** Disable before release! */
const TEST_MODE = false;

/** Test position */
const TEST_POSITION = { coords: { latitude: 51.74, longitude: 0.48 } };

setupClay();

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

    const { current, daily } = await fetchWeatherForecast(latitude, longitude);
    const {
      temperature_2m: currentTemp,
      weather_code: currentCode,
    } = current;
    const { sunrise, sunset } = daily;

    const sunriseTime = getTime(sunrise[0]);
    const sunsetTime = getTime(sunset[0]);

    const dict = {
      CURRENT_TEMP: Math.round(currentTemp),
      CURRENT_CODE: currentCode,
      SUNRISE: sunriseTime,
      SUNSET: sunsetTime,
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
