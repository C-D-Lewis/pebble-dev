import { setupClay } from './clay.js';
import {
  fetchWeatherForecast,
  getBatteryLevel,
  getLocationAsync,
  getTimeStr,
  zeroPadTo99
} from './util.js';

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
 * Get location and weather, and send to watch.
 */
const getWeatherDict = async (latitude: number, longitude: number) => {
  const { current, daily, hourly } = await fetchWeatherForecast(latitude, longitude);
  const { temperature_2m: temp2m, weather_code: currentCode } = current;
  const {
    sunrise,
    sunset,
    moon_phase: moonPhase,
    uv_index_max: uvIndexMax,
  } = daily;
  const {
    precipitation_probability: hourlyPrecip,
    weather_code: hourlyCodes,
    temperature_2m: hourlyTemps,
  } = hourly;
  const currentTemp = TEST_MODE ? TEST_CURRENT_TEMP : temp2m;
  const precipArr = TEST_MODE ? TEST_PRECIP_ARR : hourlyPrecip;
  const codesArr = TEST_MODE ? TEST_CODES_ARR : hourlyCodes;
  const tempsArr = TEST_MODE ? TEST_TEMPS_ARR : hourlyTemps;

  // Need either the next 6, or the last six of the day
  const startHour = Math.min(new Date().getHours(), precipArr.length - 6);
  const nextSixPrecip = precipArr.slice(startHour, startHour + 6);
  const nextSixCodes = codesArr.slice(startHour, startHour + 6);

  const sunriseTime = getTimeStr(sunrise[0]);
  const sunsetTime = getTimeStr(sunset[0]);

  // Derive low/high from array so it never exceeds
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

  return {
    CURRENT_TEMP: Math.round(currentTemp),
    CURRENT_CODE: currentCode,
    SUNRISE: sunriseTime,
    SUNSET: sunsetTime,
    PRECIP_ARR: nextSixPrecip.map(zeroPadTo99).join(''),
    CODE_ARR: nextSixCodes.map(zeroPadTo99).join(''),
    TEMP_LOW: Math.round(tempLow),
    TEMP_HIGH: Math.floor(tempHigh),
    MOON_PHASE: moonPhase[0] * 100,
    UV_INDEX_MAX: Math.max(1, Math.round(uvIndexMax[0])),  // Reporting 0 is silly
  };
};

/**
 * Get phone-related dict values.
 */
const getPhoneDict = async () => {
  let batteryLevel = await getBatteryLevel();
  if (TEST_MODE) {
    batteryLevel = TEST_BATTERY_LEVEL;
  }

  return {
    PHONE_BATTERY: batteryLevel,
  };
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  // Send ready signal, watch sends request with specific complications
  await PebbleTS.sendAppMessage({ READY: 1 });
});

Pebble.addEventListener('appmessage', async (e) => {
  const { payload: dict } = e;
  console.log(`AppMessage: ${JSON.stringify(dict, null, 2)}`);

  const sendPhoneDict = !!Object.values(dict).find((v) => v === 'PHONE_BATTERY');

  try {
    const {
      coords: { latitude, longitude },
    } = TEST_MODE ? TEST_POSITION : (await getLocationAsync() as GeolocationPosition);
    console.log(`Got location: ${latitude}, ${longitude}`);

    const dict = {
      ...(await getWeatherDict(latitude, longitude)),
      ...(sendPhoneDict ? await getPhoneDict() : {}),
      // ...marineDict?
    };
    console.log(JSON.stringify(dict, null, 2));

    await PebbleTS.sendAppMessage(dict);
    console.log('Weather data sent to watch');
  } catch (err: unknown) {
    console.log('Error fetching weather:');
    console.log(err);

    if (err instanceof Error) {
      console.log('Error string: ' + JSON.stringify(err));
    } else {
      // Location error?
      const e = err as GeolocationPositionError;
      if (e.code || e.message) {
        console.log('Error Code: ' + e.code);
        console.log('Error Msg: ' + e.message);
      }
    }

    await PebbleTS.sendAppMessage({ DATA_ERROR: 1 });
  }
});
