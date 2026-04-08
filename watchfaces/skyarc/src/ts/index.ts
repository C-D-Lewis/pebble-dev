/** Weather response format */
type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  hourly: {
    time: string[];
    temperature_2m: number[];
    weather_code: number[];
  };
};

/** Signed offset to allow negative numbers */
const SIGNED_OFFSET = 50;

/** Sample temperate temperatures, low -2, high 12 */
const TEST_HOURLY_TEMPS = [4, 3, 2, 1, 0, -1, -2, 0, 2, 4, 6, 8, 10, 12, 11, 9, 7, 5, 4, 3, 2, 1, 0, -1];
/** Test position */
const TEST_POSITION = { coords: { latitude: 51.74, longitude: 0.48 } };

/**
 * Zero pad a number
 */
const zeroPad = (num: number) => String(num).padStart(2, '0');

/**
 * Encode number to two digits, adding 50 to handle negative values, which
 * will be subtracted on the watch.
 */
const encodeNumber = (num: number) => {
  const val = Math.min(Math.max(Math.round(num) + SIGNED_OFFSET, 0), 99);
  return val >= 10 ? String(val) : `0${val}`;
};

/**
 * Fetch weather for this day from OpenMeteo.
 */
const fetchWeatherForecast = async (lat: number, lon: number): Promise<WeatherApiResponse> => {
  // TODO: Make configurable
  const tempUnit = 'celsius';

  const params = {
    latitude: lat,
    longitude: lon,
    hourly: 'temperature_2m,weather_code',
    current: 'temperature_2m,weather_code',
    forecast_days: 1,
    temperature_unit: tempUnit,
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
    { timeout: 5000, maximumAge: 60000, enableHighAccuracy: false },
  );
});

/**
 * Get location and weather, and send to watch.
 */
const sendWeather = async () => {
  try {
    const pos = await getLocation() as GeolocationPosition;
    // const pos = TEST_POSITION;
    const { latitude, longitude } = pos.coords;
    console.log(`Got location: ${latitude}, ${longitude}`);

    const { current, hourly } = await fetchWeatherForecast(latitude, longitude);
    const { temperature_2m: currentTemp, weather_code: currentCode } = current;
    const { time, temperature_2m: hourlyTemps, weather_code: hourlyCodes } = hourly;
    console.log(`Time range: ${time[0]} - ${time[time.length - 1]}`);

    /**
     * Arrays are two chars per item, 24 items:
     *   "CODES": "030303030303030302030202030202030302000100000000"
     */
    const dict = {
      CURRENT_TEMP: Math.round(currentTemp),
      CURRENT_CODE: currentCode,
      TEMP_ARR: hourlyTemps.map(encodeNumber).join(''),
      // TEMP_ARR: TEST_HOURLY_TEMPS.map(encodeNumber).join(''),
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
