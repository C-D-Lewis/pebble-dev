#include "test.h"

// Use local data in emulator until PKA is emulated
void test_populate_data() {
  data_set_toggle_order("012345678");
  data_set_is_lollipop(true);

  data_set_toggle_state(ToggleTypeWifi, ToggleStateOn);
  data_set_toggle_state(ToggleTypeData, ToggleStateOff);
  data_set_toggle_state(ToggleTypeBluetooth, ToggleStateOn);
  data_set_toggle_state(ToggleTypeRinger, ToggleStateSilent);
  data_set_toggle_state(ToggleTypeSync, ToggleStateOn);
  data_set_toggle_state(ToggleTypeWifiAP, ToggleStateOn);
  data_set_toggle_state(ToggleTypeFindPhone, ToggleStateOn);
  data_set_toggle_state(ToggleTypeLockPhone, ToggleStateWaiting);
  data_set_toggle_state(ToggleTypeAutoBrightness, ToggleStateBrightnessAutomatic);

  data_set_gsm_name("T-Mobile");
  data_set_wifi_name("BTHub3-NCNR");
  data_set_gsm_percent(50);
  data_set_battery_percent(75);
  data_set_storage_gb_major(16);
  data_set_storage_gb_minor(2);
  data_set_storage_percent(65);
  
  unified_window_push();
  unified_window_set_interaction_enabled(true);
  splash_window_remove_from_stack();
}
