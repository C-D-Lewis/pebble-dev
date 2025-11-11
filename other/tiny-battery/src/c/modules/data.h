#pragma once

#include <pebble.h>

// Hours between wakeups
// #define WAKEUP_INTERVAL_S (60 * 60 * 6) // 6 hours
#define WAKEUP_INTERVAL_S (60 * 2) // 2 mins

// If there is no data yet for this value
#define NO_DATA -1

typedef enum {
  // Last time we stopped charging
  SK_DischargeStartTime,
  // Last time we updated
  SK_LastUpdateTime,
  // Last charge percent
  SK_LastChargePerc,
  // Current discharge rate
  SK_DischargeRate,
  // ID of the scheduled wakeup
  SK_WakeupId,
  // If we were charging on last update
  SK_WasPlugged,

  // Base for stored discharge rates (previous 10)
  SK_PreviousRatesBase = 100,

  // Max storage value used
  SK_Max = 110
} StorageKey;

void data_init();
void data_deinit();

void data_prepare();

void data_log_state();

int data_get_discharge_start_time(void);
void data_set_discharge_start_time(int time);

int data_get_last_update_time(void);
void data_set_last_update_time(int time);

int data_get_last_charge_perc(void);
void data_set_last_charge_perc(int perc);

int data_get_discharge_rate(void);
void data_set_discharge_rate(int rate);

int data_get_wakeup_id();
void data_set_wakeup_id(int id);

bool data_get_was_plugged();
void data_set_was_plugged(bool b);
