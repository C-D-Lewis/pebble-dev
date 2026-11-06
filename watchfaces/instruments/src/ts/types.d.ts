/** Weather response format */
export type WeatherApiResponse = {
  current: {
    temperature_2m: number;
    weather_code: number;
  };
  daily: {
    sunrise: string[];
    sunset: string[];
    moon_phase: number[];
  };
  hourly: {
    precipitation_probability: number[];
    weather_code: number[];
    temperature_2m: number[];
  };
};

// Stub navigator battery API
declare global {
  interface Navigator {
    getBattery(): Promise<{ level: number }>;
  }
}
