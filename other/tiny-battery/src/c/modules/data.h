#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/error_window.h"

// If there is no data yet for this value
#define DATA_EMPTY -1

typedef enum {
  // Last time we stopped charging
  SK_DischargeStartTime,
  // Last time we updated
  SK_LastUpdateTime,
  // Last charge percent
  SK_LastChargePerc,
  // ID of the scheduled wakeup
  SK_WakeupId,
  // If we were charging on last update
  SK_WasPlugged,
  // Recent discharge rate values
  SK_SampleData,

  // Max storage value used
  SK_Max = 10
} StorageKey;

typedef struct {
  // History of discharge rate values used for averaging
  int history[NUM_STORED_SAMPLES];
} SampleData;

void data_init();
void data_deinit();

void data_prepare();

void data_log_state();

void data_push_sample_value(int v);

int data_get_history_avg_rate();

int data_calculate_days_remaining();

void data_sample_now();

int data_get_discharge_start_time(void);
void data_set_discharge_start_time(int time);

int data_get_last_update_time(void);
void data_set_last_update_time(int time);

int data_get_last_charge_perc(void);
void data_set_last_charge_perc(int perc);

int data_get_wakeup_id();
void data_set_wakeup_id(int id);

bool data_get_was_plugged();
void data_set_was_plugged(bool b);

SampleData* data_get_sample_data();

void data_set_error(char *err);
char* data_get_error();
