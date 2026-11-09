import { setupClay } from './clay.js';
import { WeatherApiResponse } from './types.js';

/** Disable before release! */
const TEST_MODE = false;
/** Test position */
const TEST_POSITION = { coords: { latitude: 51.74, longitude: 0.48 } };
/** Sample precipitation data */
const TEST_PRECIP_ARR = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 40, 20, 50, 90, 20];
/** Sample weather codes */
const TEST_CODES_ARR = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 77, 82, 99];
/** Test current temp */
const TEST_CURRENT_TEMP = 12;
/** Sample temperatures */
const TEST_TEMPS_ARR = [10, 11, 12, 13, 14, 15, 14, 13, 12, 11, 10, 11, 12, 13, 14, 15, 14, 13, 12, 11, 10, 11, 12, 13];
/** Test phone battery level */
const TEST_BATTERY_LEVEL = 75;

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
    daily: 'sunrise,sunset,moon_phase',
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
    {
      timeout: 5000,
      maximumAge: 60000,
      enableHighAccuracy: false,
    },
  );
});

/**
 * Get the time portion of ISO datetime in format HH:MM, e.g. 06:12
 */
const getTime = (str: string) => str.split('T')[1];

/**
 * Get progress to next month.
 * Much easier to calculate in JS than C.
 */
const getMonthProgress = () => {
  const now = new Date();
  const nextMonthDays = new Date(now.getFullYear(), now.getMonth() + 1, 0).getDate();
  return Math.round((now.getDate() / nextMonthDays) * 100);
};

/**
 * Get progress through the year.
 */
const getYearProgress = () => {
  const now = new Date();
  const dayMs = 24 * 60 * 60 * 1000;
  const year = now.getFullYear();

  const dayOfYear = (Date.UTC(year, now.getMonth(), now.getDate()) - Date.UTC(year, 0, 1)) / dayMs + 1;
  const daysInYear = (Date.UTC(year + 1, 0, 1) - Date.UTC(year, 0, 1)) / dayMs;

  return Math.round((dayOfYear / daysInYear) * 100);
};

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
      temperature_2m: temp2m,
      weather_code: currentCode,
    } = current;
    const { sunrise, sunset, moon_phase: moonPhase } = daily;
    const {
      precipitation_probability: hourlyPrecip,
      weather_code: hourlyCodes,
      temperature_2m: hourlyTemps,
    } = hourly;

    const sunriseTime = getTime(sunrise[0]);
    const sunsetTime = getTime(sunset[0]);

    const currentTemp = TEST_MODE ? TEST_CURRENT_TEMP : temp2m;

    // Need either the next 6, or the last six of the day
    const precipArr = TEST_MODE ? TEST_PRECIP_ARR : hourlyPrecip;
    const startHour = Math.min(new Date().getHours(), precipArr.length - 6);
    const nextSixPrecip = precipArr.slice(startHour, startHour + 6);
    const codesArr = TEST_MODE ? TEST_CODES_ARR : hourlyCodes;
    const nextSixCodes = codesArr.slice(startHour, startHour + 6);

    // Derive low/high from array so it never exceeds
    const tempsArr = TEST_MODE ? TEST_TEMPS_ARR : hourlyTemps;
    let tempLow = 999;
    let tempHigh = -999;
    tempsArr.forEach((i) => {
      if (i < tempLow) {
        tempLow = i;
      }
      if (i > tempHigh) {
        tempHigh = i;
      }
    });

    // Get device battery
    let batteryLevel = 0;
    if (!navigator.getBattery) {
      console.log('navigator.getBattery not supported');
      batteryLevel = 0;
    } else {
      const b = await navigator.getBattery();
      batteryLevel = Math.round(b.level * 100);
    }
    if (TEST_MODE) {
      batteryLevel = TEST_BATTERY_LEVEL;
    }

    const dict = {
      CURRENT_TEMP: Math.round(currentTemp),
      CURRENT_CODE: currentCode,
      SUNRISE: sunriseTime,
      SUNSET: sunsetTime,
      PRECIP_ARR: nextSixPrecip.map(zeroPadTo99).join(''),
      CODE_ARR: nextSixCodes.map(zeroPadTo99).join(''),
      TEMP_LOW: Math.round(tempLow),
      TEMP_HIGH: Math.floor(tempHigh),
      PHONE_BATTERY: batteryLevel,
      MONTH_PROGRESS: getMonthProgress(),
      YEAR_PROGRESS: getYearProgress(),
      MOON_PHASE: moonPhase[0] * 100,
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

    await PebbleTS.sendAppMessage({ DATA_ERROR: 1 });
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
