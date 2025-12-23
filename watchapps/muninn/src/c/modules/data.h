#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/alert_window.h"

// If there is no data yet for this value - can't change, already in persist
#define STATUS_EMPTY -1
// If there was no change from the last sample
#define STATUS_NO_CHANGE -102
// If this sample looked like it was higher than the last
#define STATUS_CHARGED -103

// NOTE: ADD ONLY - Changing keys will affect existing users!
typedef enum {
  // App data key
  SK_AppData = 1,
  // Sample data key base - due to the 256B limit per-key we need to store separately
  SK_SampleBase = 10,
  // Wipe - adding new fields to Sample struct and expanding number of samples
  SK_Migration_1 = 50,

  // Max storage value used
  SK_Max = 100
} StorageKey;

typedef enum {
  AL_OFF = STATUS_EMPTY,
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
  bool push_timeline_pins;
  bool elevated_rate_alert;

  // Singleton, adding new fields OK
} AppData;

// A full wakeup sample
typedef struct {
  int timestamp;        // This sample
  int charge_perc;      //
  int last_sample_time; // Previous values used in the calculation
  int last_charge_perc; //
  int time_diff;        // Comparison values
  int charge_diff;      //
  int result;           // Result - or special status!  
  int days_remaining;   // Interesting for analysis
  int rate;             //

  // Saved per sample, adding new fields okay
} Sample;

// Methods
void data_init();
void data_deinit();
void data_reset_all();
void data_log_state();
void data_push_sample(int charge_perc, int last_sample_time, int last_charge_perc, int time_diff, int charge_diff, int result);
int data_calculate_avg_discharge_rate();
int data_calculate_days_remaining();
void data_cycle_custom_alert_level();
bool data_get_rate_is_elevated();

// Interface getters/setters
int data_get_last_sample_time(void);
void data_set_last_sample_time(int time);
int data_get_last_charge_perc(void);
void data_set_last_charge_perc(int perc);
int data_get_wakeup_id();
void data_set_wakeup_id(int id);
Sample* data_get_sample(int index);
void data_set_error(char *err);
char* data_get_error();
void data_set_seen_first_launch();
bool data_get_seen_first_launch();
bool data_get_vibe_on_sample();
void data_set_vibe_on_sample(bool v);
int data_get_custom_alert_level();
int data_get_valid_samples_count();
int data_get_log_length();
bool data_get_ca_has_notified();
void data_set_ca_has_notified(bool notified);
bool data_get_push_timeline_pins();
void data_set_push_timeline_pins(bool b);
bool data_get_elevated_rate_alert();
void data_set_elevated_rate_alert(bool b);

// Strings
#define MSG_WELCOME "Welcome to Muninn!\n\nEstimates will appear after two samples are taken.\n\nPlease launch me if the watch is off and a sample is missed."
#define MSG_ABOUT "Odin tasked Muninn with memory of the land...\n\nHe wakes every 6 hours to note the battery level.\n\nOver time, he will provide you with battery wisdom."
#define MSG_TIPS "Use a watchface that updates every minute.\n\nFilter notifications from very noisy apps.\n\nDisable the motion activated backlight."
