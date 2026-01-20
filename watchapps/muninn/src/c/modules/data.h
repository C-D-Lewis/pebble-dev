#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/message_window.h"

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
  SK_Max = 51
} StorageKey;

typedef enum {
  AL_OFF = STATUS_EMPTY,
  AL_50 = 50,
  AL_20 = 20,
  AL_10 = 10,
} AlertLevel;

// Persisted app data - be careful changing this!
typedef struct {
  int last_sample_time;     // Last sample timestamp
  int last_charge_perc;     // Last sample charge percentage
  int wakeup_id;            // Current wakeup ID
  bool seen_first_launch;   // Has the app been launched before?
  bool vibe_on_sample;      // Vibrate on sample
  int custom_alert_level;   // Custom alert level
  bool ca_has_notified;     // Has notified for custom alert
  bool push_timeline_pins;  // Push timeline pins
  bool elevated_rate_alert; // Elevated rate alert
  int _unused_pin_set_time; 
  bool one_day_notified;    // One day remaining alert has notified
  int last_charge_time;     // Timestamp of last charge
  bool one_day_alert;       // Show 'one day remaining' alert

  // Singleton, adding new fields OK, removing from middle is NOT OK
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
void data_reset_log();
time_t data_get_next_charge_time();
int data_calculate_accuracy();
int data_calculate_days_remaining_accuracy();

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
bool data_get_one_day_notified();
void data_set_one_day_notified(bool b);
void data_set_last_charge_time(int ts);
int data_get_last_charge_time();
bool data_get_one_day_alert();
void data_set_one_day_alert(bool b);

// Strings
#define MSG_WELCOME "Welcome to Muninn!\n\nEstimates will appear after two samples are taken.\n\nPlease launch me if the watch is off and a sample is missed."
#define MSG_TIPS "Use a watchface that updates every minute.\n\nFilter notifications from very noisy apps.\n\nDisable the motion activated backlight and lower intensity."
#define MSG_INFORMATION "Samples are taken every 6 hours.\n\nThe main screen shows:\n- Estimate data if available.\n- Last detected & next predicted charge.\n- Current battery level and next sample time.\n\nThe graph shows each reading, with an outlined circle for predicted levels.\n\nIf watch is off when a wakeup is missed, the app must be launched again."