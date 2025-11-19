#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/alert_window.h"

// If there is no data yet for this value
#define DATA_EMPTY -1

// NOTE: ADD ONLY - Changing keys will affect existing users!
typedef enum {
  // Last time we stopped charging
  SK_DischargeStartTime = 0,
  // Last time we updated
  __SK_LastUpdateTime = 1,
  // Last charge percent
  SK_LastChargePerc = 2,
  // ID of the scheduled wakeup
  SK_WakeupId = 3,
  // If we were charging on last update
  SK_WasPlugged = 4,
  // Recent discharge rate values
  SK_SampleData = 5,
  // Set on first launch for intro
  SK_SeenFirstLaunch = 6,
  // Vibrate when a sample is taken
  SK_VibeOnSample = 7,
  // Custom alert level value
  SK_CustomAlertLevel = 8,

  // Max storage value used
  SK_Max = 25
} StorageKey;

typedef enum {
  AL_OFF = DATA_EMPTY,
  AL_50 = 50,
  AL_20 = 20,
  AL_10 = 10,
} AlertLevel;

// History of discharge rate values used for averaging
typedef struct {
  int timestamps[NUM_STORED_SAMPLES];
  int values[NUM_STORED_SAMPLES];
} SampleData;

void data_reset_all();

void data_init();
void data_deinit();

void data_log_state();

void data_initial_sample();

void data_push_sample_value(int v);

int data_get_history_avg_rate();

int data_calculate_days_remaining();

int data_get_discharge_start_time(void);
void data_set_discharge_start_time(int time);

int data_get_last_charge_perc(void);
void data_set_last_charge_perc(int perc);

int data_get_wakeup_id();
void data_set_wakeup_id(int id);

bool data_get_was_plugged();
void data_set_was_plugged(bool b);

SampleData* data_get_sample_data();

void data_set_error(char *err);
char* data_get_error();

void data_set_seen_first_launch();
bool data_get_seen_first_launch();

bool data_get_vibe_on_sample();
void data_set_vibe_on_sample(bool v);

int data_get_custom_alert_level();
void data_cycle_custom_alert_level();

int data_get_samples_count();
