/** Weather response format */
type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  hourly: {
    time: string[];
    temperature_2m: number[];
    precipitation_probability: number[];
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
  // TODO: Configurable
  const windUnit = 'mph';
  const tempUnit = 'celsius';

  const url = `https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lon}&hourly=temperature_2m,precipitation_probability,weather_code&current=temperature_2m,weather_code&forecast_days=1&wind_speed_unit=${windUnit}&temperature_unit=${tempUnit}&timezone=auto`;
  const res = await fetch(url);
  if (res.status !== 200) throw new Error(await res.text());
  
  const json = await res.json() as WeatherApiResponse;
  // console.log(JSON.stringify(json, null, 2));
  return json;
};

/**
 * Get current location.
 */
const getLocation = async () => new Promise((resolve, reject) => {
  navigator.geolocation.getCurrentPosition(
    resolve,
    reject,
    { timeout: 10000, maximumAge: 60000 },
  );
});

/**
 * Get location and weather, and send to watch.
 */
const sendWeather = async () => {
  const location = await getLocation() as GeolocationPosition;
  const { latitude, longitude } = location.coords;
  console.log(`Got location: ${latitude}, ${longitude}`);

  const weather = await fetchWeatherForecast(latitude, longitude);
  const { current, hourly } = weather;
  const { temperature_2m: currentTemp, weather_code: currentCode } = current;
  const {
    time,
    temperature_2m: hourlyTemps,
    // precipitation_probability: hourlyPrecip,
    weather_code: hourlyCodes,
  } = hourly;

  console.log(`Time range: ${time[0]} - ${time[time.length - 1]}`);

  /**
   * Arrays are two chars per item, 24 items.
   *
   * {
   *   "CURRENT_TEMP": 12,
   *   "CURRENT_CODE": 2,
   *   "TEMPS": "121211101009080910101112121212121211100807060606",
   *   "PRECIP": "000000000000000000020713161408040201000000000000",
   *   "CODES": "030303030303030302030202030202030302000100000000"
   * }
   */
  const dict = {
    CURRENT_TEMP: Math.round(currentTemp),
    CURRENT_CODE: currentCode,
    TEMP_ARR: hourlyTemps.reduce((acc, p) => acc + zeroPad(Math.round(p)), ''),
    CODE_ARR: hourlyCodes.reduce((acc, p) => acc + zeroPad(p), ''),
    // Unused
    // PRECIP_ARR: hourlyPrecip.reduce((acc, p) => acc + zeroPad(Math.round(p)), ''),
  };
  console.log(JSON.stringify(dict, null, 2));
  
  // Send to watch
  await PebbleTS.sendAppMessage(dict);
  console.log('Weather data sent to watch');
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  try {
    sendWeather();
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
