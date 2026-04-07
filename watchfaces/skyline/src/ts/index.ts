/** Weather response format */
type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  hourly: {
    time: string[];
    temperature_2m: number[];
    precipitation_probability: number[]; // Unused
    weather_code: number[];
  };
};

/**
 * Pad number to two digits.
 */
const zeroPad = (num: number) => String(num).padStart(2, '0');

/**
 * Fetch weather for this day from OpenMeteo.
 */
const fetchWeatherForecast = async (lat: number, lon: number): Promise<WeatherApiResponse> => {
  // TODO: Make configurable
  const windUnit = 'mph';
  const tempUnit = 'celsius';

  const params = {
    latitude: lat,
    longitude: lon,
    hourly: 'temperature_2m,precipitation_probability,weather_code',
    current: 'temperature_2m,weather_code',
    forecast_days: 1,
    wind_speed_unit: windUnit,
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
  const pos = await getLocation() as GeolocationPosition;
  const { latitude, longitude } = pos.coords;
  console.log(`Got location: ${latitude}, ${longitude}`);

  const { current, hourly } = await fetchWeatherForecast(latitude, longitude);
  const { temperature_2m: currentTemp, weather_code: currentCode } = current;
  const { time, temperature_2m: hourlyTemps, weather_code: hourlyCodes } = hourly;
  console.log(`Time range: ${time[0]} - ${time[time.length - 1]}`);

  /**
   * Arrays are two chars per item, 24 items:
   *   "TEMPS": "121211101009080910101112121212121211100807060606"
   *   "CODES": "030303030303030302030202030202030302000100000000"
   */
  const dict = {
    CURRENT_TEMP: Math.round(currentTemp),
    CURRENT_CODE: currentCode,
    TEMP_ARR: hourlyTemps.map((p) => zeroPad(Math.round(p))).join(''),
    CODE_ARR: hourlyCodes.map((p) => zeroPad(p)).join(''),
  };
  console.log(JSON.stringify(dict, null, 2));
  
  // Send to watch
  await PebbleTS.sendAppMessage(dict);
  console.log('Weather data sent to watch');
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  try {
    await sendWeather();
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});

Pebble.addEventListener('appmessage', async (e) => {
  const { payload: dict } = e;
  console.log(`appmessage: ${JSON.stringify(dict)}`);

  try {
    await sendWeather();
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
