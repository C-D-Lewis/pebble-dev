/** Weather response format */
export type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  daily: {
    sunrise: string[];
    sunset: string[];
  };
  hourly: {
    precipitation_probability: number[];
    weather_code: number[];
    temperature_2m: number[];
  };
};
