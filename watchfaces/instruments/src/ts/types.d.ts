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
    uv_index_max: number[];
  };
  hourly: {
    precipitation_probability: number[];
    weather_code: number[];
    temperature_2m: number[];
  };
};

export type MarineApiResponse = {
  
}

// Stub navigator battery API
declare global {
  interface Navigator {
    getBattery(): Promise<{ level: number }>;
  }
}
