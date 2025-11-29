#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/alert_window.h"

// If there is no data yet for this value
#define DATA_EMPTY -1

// NOTE: ADD ONLY - Changing keys will affect existing users!
typedef enum {
  // App data key
  SK_AppData = 0,
  // Sample data key
  SK_SampleData = 1,

  // Max storage value used
  SK_Max = 25
} StorageKey;

typedef enum {
  AL_OFF = DATA_EMPTY,
  AL_50 = 50,
  AL_20 = 20,
  AL_10 = 10,
} AlertLevel;

// Persisted app data - be careful changing this!
typedef struct {
  int last_sample_time;
  int last_charge_perc;
  int wakeup_id;
  bool seen_first_launch;
  bool vibe_on_sample;
  int custom_alert_level;
  bool ca_has_notified;
} AppData;

// Capture more data for debugging purposes
typedef struct {
  // This sample
  int timestamp;
  int charge_perc;

  // Previous values used in the calculation
  int last_sample_time;
  int last_charge_perc;
  
  // Comparison values
  int time_diff;
  int charge_diff;

  // Result
  int perc_per_day;
} Sample;

// History of discharge rate values used for averaging
typedef struct {
  Sample samples[NUM_STORED_SAMPLES];
} SampleData;

// Methods
void data_init();
void data_deinit();
void data_reset_all();
void data_log_state();
void data_activation_update();
void data_push_sample(int charge_perc, int last_sample_time, int last_charge_perc, int time_diff, int charge_diff, int perc_per_day);
int data_calculate_avg_discharge_rate();
int data_calculate_days_remaining();
void data_cycle_custom_alert_level();

// Interface getters/setters
int data_get_last_sample_time(void);
void data_set_last_sample_time(int time);
int data_get_last_charge_perc(void);
void data_set_last_charge_perc(int perc);
int data_get_wakeup_id();
void data_set_wakeup_id(int id);
SampleData* data_get_sample_data();
void data_set_error(char *err);
char* data_get_error();
void data_set_seen_first_launch();
bool data_get_seen_first_launch();
bool data_get_vibe_on_sample();
void data_set_vibe_on_sample(bool v);
int data_get_custom_alert_level();
int data_get_samples_count();
bool data_get_ca_has_notified();
void data_set_ca_has_notified(bool notified);

// Strings
#define MSG_WELCOME "Welcome to Muninn!\n\nEstimates will appear after two meaningful samples.\n\nPlease launch me if the watch is off during a 6h interval."
#define MSG_ABOUT "Odin tasked Muninn with memory of the land...\n\nHe wakes every 6 hours to note the battery level.\n\nOver time, he will provide you with battery wisdom."
#define MSG_TIPS "Use a watchface that updates every minute.\n\nFilter notifications from very noisy apps.\n\nDisable the motion activated backlight."
