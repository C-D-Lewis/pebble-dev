import { WeatherApiResponse } from './types';

/**
 * Zero pad a number, max 99
 */
export const zeroPadTo99 = (num: number) => {
  if (num < 10) return `0${num}`;
  if (num > 99) return '99';
  return String(num);
};

/**
 * Get phone battery level, if supported.
 */
export const getBatteryLevel = async (): Promise<number> => {
  if (!navigator.getBattery) {
    console.log('navigator.getBattery not supported');
    return 0;
  } else {
    const b = await navigator.getBattery();
    return Math.round(b.level * 100);
  }
};

/**
 * Fetch weather for this day from OpenMeteo.
 */
export const fetchWeatherForecast = async (lat: number, lon: number ): Promise<WeatherApiResponse> => {
  const params = {
    latitude: lat,
    longitude: lon,
    hourly: 'temperature_2m,weather_code,precipitation_probability',
    current: 'temperature_2m,weather_code',
    daily: 'sunrise,sunset,moon_phase,uv_index_max',
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
export const getLocationAsync = async () => new Promise((resolve, reject) => {
  navigator.geolocation.getCurrentPosition(
    resolve,
    reject,
    {
      timeout: 5000,
      maximumAge: 60000,
      enableHighAccuracy: true,
    },
  );
});

/**
 * Get the time portion of ISO datetime in format HH:MM, e.g. 06:12
 */
export const getTimeStr = (str: string) => str.split('T')[1];
