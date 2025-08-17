#pragma once

#include <pebble.h>

#define OWM_WEATHER_BUFFER_SIZE 32

//! Possible statuses of the weather library
typedef enum {
  //! Weather library has not yet initiated a fetch
  OWMWeatherStatusNotYetFetched = 0,
  //! Bluetooth is disconnected
  OWMWeatherStatusBluetoothDisconnected,
  //! Weather data fetch is in progress
  OWMWeatherStatusPending,
  //! Weather fetch failed
  OWMWeatherStatusFailed,
  //! Weather fetched and available
  OWMWeatherStatusAvailable,
  //! API key was bad
  OWMWeatherStatusBadKey,
  //! Location not available
  OWMWeatherStatusLocationUnavailable
} OWMWeatherStatus;

//! Struct containing weather data
typedef struct {
  //! Weather conditions string e.g: "Sky is clear"
  char description[OWM_WEATHER_BUFFER_SIZE];
  //! Short conditions string e.g: "Clear"
  char description_short[OWM_WEATHER_BUFFER_SIZE];
  //! Name of the location from the weather feed
  char name[OWM_WEATHER_BUFFER_SIZE];
  //! Temperature in degrees Kelvin, Celcius, and Farenheit
  int temp_k;
  int temp_c;
  int temp_f;
  //! Date that the data was received
  time_t timestamp;
} OWMWeatherInfo;

//! Callback for a weather fetch
//! @param info The struct containing the weather data
//! @param status The current OWMWeatherStatus, which may have changed.
typedef void(OWMWeatherCallback)(OWMWeatherInfo *info, OWMWeatherStatus status);

//! Initialize the weather library. The data is fetched after calling this, and should be accessed
//! and stored once the callback returns data, if it is successful.
//! @param api_key The API key or 'appid' from your OpenWeatherMap account.
void owm_weather_init(char *api_key);

//! Important: This uses the AppMessage system. You should only use AppMessage yourself
//! either before calling this, or after you have obtained your weather data.
//! @param callback Callback to be called once the weather.
//! @return true if the fetch message to PebbleKit JS was successful, false otherwise.
bool owm_weather_fetch(OWMWeatherCallback *callback);

//! Deinitialize and free the backing OWMWeatherInfo.
void owm_weather_deinit();

//! Peek at the current state of the weather library. You should check the OWMWeatherStatus of the 
//! returned OWMWeatherInfo before accessing data members.
//! @return OWMWeatherInfo object, internally allocated. 
//! If NULL, owm_weather_init() has not been called.
OWMWeatherInfo* owm_weather_peek();
