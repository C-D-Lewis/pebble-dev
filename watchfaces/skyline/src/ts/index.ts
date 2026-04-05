/** Weather response format */
type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  hourly: {
    temperature_2m: number[];
    precipitation_probability: number[];
    weather_code: number[];
  };
};

/** Weather strings map */
const WeatherStrings = {
  Clear: 'CLEAR',
  Cloud: 'CLOUD',
  Fog: 'FOG',
  Drizzle: 'DRIZZLE',
  Rain: 'RAIN',
  Snow: 'SNOW',
  Showers: 'SHOWERS',
  Thunder: 'THUNDER',
  Unknown: 'UNKNOWN',
};

/**
 * Get parsable string from weather code groups. The watch will use these constants.
 */
const weatherCodeToString = (code: number) => {
  switch (code) {
    case 0:
      return WeatherStrings.Clear;
    case 1: // Mainly clear
    case 2: // Partly cloudy
    case 3: // Overcast
      return WeatherStrings.Cloud;
    // Fog and Rime Fog (45, 48)
    case 45: // Fog
    case 48: // Depositing rime fog
      return WeatherStrings.Fog;
    // Drizzle (51, 53, 55) & Freezing Drizzle (56, 57)
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
      return WeatherStrings.Drizzle;
    // Rain (61, 63, 65) & Freezing Rain (66, 67)
    case 61: // Slight Rain
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 66: // Light Freezing Rain
    case 67: // Heavy Freezing Rain
      return WeatherStrings.Rain;
    // Snow Fall and Snow Grains (71, 73, 75, 77)
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return WeatherStrings.Snow;
    // Rain Showers (80, 81, 82)
    case 80: // Slight Rain showers
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
      return WeatherStrings.Showers;
    // Thunderstorm (95, 96, 99)
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return WeatherStrings.Thunder;
    // Default/Unknown Code
    default:
      return WeatherStrings.Unknown;
  }
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

  const url = `https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lon}&hourly=temperature_2m,precipitation_probability,weather_code&current=temperature_2m,weather_code&forecast_days=1&wind_speed_unit=${windUnit}&temperature_unit=${tempUnit}`;
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

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  try {
    const location = await getLocation() as GeolocationPosition;
    const { latitude, longitude } = location.coords;
    console.log(`Got location: ${latitude}, ${longitude}`);

    const weather = await fetchWeatherForecast(latitude, longitude);
    const { current, hourly } = weather;
    const { temperature_2m: currentTemp, weather_code: currentCode } = current;
    const {
      temperature_2m: hourlyTemps,
      precipitation_probability: hourlyPrecip,
      weather_code: hourlyCodes,
    } = hourly;

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
      PRECIP_ARR: hourlyPrecip.reduce((acc, p) => acc + zeroPad(Math.round(p)), ''),
      CODE_ARR: hourlyCodes.reduce((acc, p) => acc + zeroPad(p), ''),
    };
    console.log(JSON.stringify(dict, null, 2));
    
    // Send to watch
    await PebbleTS.sendAppMessage(dict);
    console.log('Weather data sent to watch');
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
